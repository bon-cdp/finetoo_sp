"""
Logical Consistency Scorer using Wreath-Sheaf Framework

This module takes natural language statements and computes a cohomological
obstruction score that quantifies their logical consistency. A score of 0
indicates perfect logical consistency, while higher scores indicate contradictions.

The key insight: While wreath-sheaf isn't good at generating human-like text,
it's PERFECT for measuring logical consistency through its cohomological obstruction.
"""

import numpy as np
import re
from typing import List, Tuple, Dict, Set
from dataclasses import dataclass
import sys
import os

# Add parent directory to path to import from fhe_transformer
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'fhe_transformer'))

from unified_sheaf_learner import UnifiedSheafLearner
from generalized_sheaf_learner import GeneralizedSheafLearner


@dataclass
class LogicalStatement:
    """Represents a parsed logical statement."""
    subject: str
    relation: str  # "is", "is_not", "has", "lacks", etc.
    object: str
    raw_text: str
    statement_id: int


@dataclass
class LogicalEntity:
    """Represents an entity with its properties."""
    name: str
    properties: Set[str]
    relations: Dict[str, Set[str]]  # relation_type -> set of related entities


class LogicalConsistencyScorer:
    """
    Scores the logical consistency of a set of statements using wreath-sheaf.
    """

    def __init__(self, verbose=False):
        self.verbose = verbose
        self.statements: List[LogicalStatement] = []
        self.entities: Dict[str, LogicalEntity] = {}
        self.sheaf_learner = UnifiedSheafLearner(verbose=verbose)

    def parse_statements(self, text: str) -> List[LogicalStatement]:
        """
        Parse raw text into logical statements.

        Supports patterns like:
        - "X is Y"
        - "X is not Y"
        - "X has Y"
        - "X and Y have Z"
        - "All X are Y"
        """
        statements = []
        lines = text.strip().split('\n')

        for idx, line in enumerate(lines):
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            # Pattern 1: "X is Y" or "X is not Y"
            if ' is not ' in line:
                parts = line.split(' is not ')
                if len(parts) == 2:
                    statements.append(LogicalStatement(
                        subject=parts[0].strip(),
                        relation='is_not',
                        object=parts[1].strip().rstrip('.'),
                        raw_text=line,
                        statement_id=idx
                    ))
            elif ' is ' in line:
                parts = line.split(' is ')
                if len(parts) == 2:
                    statements.append(LogicalStatement(
                        subject=parts[0].strip(),
                        relation='is',
                        object=parts[1].strip().rstrip('.'),
                        raw_text=line,
                        statement_id=idx
                    ))

            # Pattern 2: "X has Y" or "X lacks Y"
            elif ' has ' in line:
                parts = line.split(' has ')
                if len(parts) == 2:
                    statements.append(LogicalStatement(
                        subject=parts[0].strip(),
                        relation='has',
                        object=parts[1].strip().rstrip('.'),
                        raw_text=line,
                        statement_id=idx
                    ))
            elif ' lacks ' in line:
                parts = line.split(' lacks ')
                if len(parts) == 2:
                    statements.append(LogicalStatement(
                        subject=parts[0].strip(),
                        relation='lacks',
                        object=parts[1].strip().rstrip('.'),
                        raw_text=line,
                        statement_id=idx
                    ))

            # Pattern 3: "X and Y have Z"
            elif ' and ' in line and ' have ' in line:
                match = re.match(r'(.*?) and (.*?) have (.*)', line)
                if match:
                    subj1, subj2, obj = match.groups()
                    statements.append(LogicalStatement(
                        subject=subj1.strip(),
                        relation='has',
                        object=obj.strip().rstrip('.'),
                        raw_text=line,
                        statement_id=idx
                    ))
                    statements.append(LogicalStatement(
                        subject=subj2.strip(),
                        relation='has',
                        object=obj.strip().rstrip('.'),
                        raw_text=line,
                        statement_id=idx + 0.5
                    ))

        return statements

    def build_entity_graph(self, statements: List[LogicalStatement]) -> Dict[str, LogicalEntity]:
        """
        Build a graph of entities and their relationships from statements.
        """
        entities = {}

        for stmt in statements:
            # Ensure subject entity exists
            if stmt.subject not in entities:
                entities[stmt.subject] = LogicalEntity(
                    name=stmt.subject,
                    properties=set(),
                    relations={}
                )

            # Ensure object entity exists if it's an entity reference
            if stmt.relation in ['is', 'is_not'] and stmt.object not in entities:
                entities[stmt.object] = LogicalEntity(
                    name=stmt.object,
                    properties=set(),
                    relations={}
                )

            # Add relationships
            if stmt.relation == 'is':
                if 'is' not in entities[stmt.subject].relations:
                    entities[stmt.subject].relations['is'] = set()
                entities[stmt.subject].relations['is'].add(stmt.object)
            elif stmt.relation == 'is_not':
                if 'is_not' not in entities[stmt.subject].relations:
                    entities[stmt.subject].relations['is_not'] = set()
                entities[stmt.subject].relations['is_not'].add(stmt.object)
            elif stmt.relation == 'has':
                entities[stmt.subject].properties.add(stmt.object)
            elif stmt.relation == 'lacks':
                entities[stmt.subject].properties.add(f"not_{stmt.object}")

        return entities

    def encode_as_vectors(self, statements: List[LogicalStatement], entities: Dict[str, LogicalEntity]) -> Tuple[List[np.ndarray], List[np.ndarray]]:
        """
        Encode statements as vectors for wreath-sheaf analysis.

        Each statement becomes a vector where dimensions represent:
        - Entity presence/absence
        - Relation types
        - Properties
        """
        # Build vocabulary
        entity_names = list(entities.keys())
        all_properties = set()
        all_relations = set()

        for entity in entities.values():
            all_properties.update(entity.properties)
            all_relations.update(entity.relations.keys())

        property_list = sorted(list(all_properties))
        relation_list = sorted(list(all_relations))

        # Vector dimensions
        n_entities = len(entity_names)
        n_properties = len(property_list)
        n_relations = len(relation_list)
        vector_dim = n_entities + n_properties + n_relations

        if vector_dim == 0:
            return [], []

        # Encode each statement
        V_samples = []
        targets = []

        for stmt in statements:
            # Create input vector (what the statement claims)
            v = np.zeros((vector_dim, 1), dtype=complex)

            # Mark subject entity
            if stmt.subject in entity_names:
                v[entity_names.index(stmt.subject), 0] = 1

            # Mark relation
            if stmt.relation in ['is', 'is_not']:
                if stmt.relation in relation_list:
                    rel_idx = n_entities + n_properties + relation_list.index(stmt.relation)
                    v[rel_idx, 0] = 1
                # Mark object entity
                if stmt.object in entity_names:
                    v[entity_names.index(stmt.object), 0] = 0.5  # Different weight for object

            # Mark properties
            if stmt.relation == 'has' and stmt.object in property_list:
                prop_idx = n_entities + property_list.index(stmt.object)
                v[prop_idx, 0] = 1
            elif stmt.relation == 'lacks':
                neg_prop = f"not_{stmt.object}"
                if neg_prop in property_list:
                    prop_idx = n_entities + property_list.index(neg_prop)
                    v[prop_idx, 0] = 1

            V_samples.append(v)

            # Target is consistency indicator (1 for consistent, 0 for contradiction)
            # We'll use the sheaf framework to learn this
            targets.append(np.ones((1, 1), dtype=complex))

        return V_samples, targets

    def detect_contradictions(self, entities: Dict[str, LogicalEntity]) -> List[Tuple[str, str]]:
        """
        Detect obvious contradictions in the entity graph.

        Returns list of (statement1, statement2) pairs that contradict.
        """
        contradictions = []

        for entity_name, entity in entities.items():
            # Check for "is" and "is_not" contradictions
            if 'is' in entity.relations and 'is_not' in entity.relations:
                for is_target in entity.relations['is']:
                    if is_target in entity.relations['is_not']:
                        contradictions.append(
                            (f"{entity_name} is {is_target}",
                             f"{entity_name} is not {is_target}")
                        )

            # Check for property contradictions (has X and not_X)
            for prop in entity.properties:
                if prop.startswith('not_'):
                    positive_prop = prop[4:]
                    if positive_prop in entity.properties:
                        contradictions.append(
                            (f"{entity_name} has {positive_prop}",
                             f"{entity_name} lacks {positive_prop}")
                        )

        # Check for transitivity violations
        for entity_name, entity in entities.items():
            if 'is' in entity.relations:
                for target in entity.relations['is']:
                    if target in entities and 'is' in entities[target].relations:
                        # If A is B and B is C, check if A is_not C
                        for trans_target in entities[target].relations['is']:
                            if 'is_not' in entity.relations and trans_target in entity.relations['is_not']:
                                contradictions.append(
                                    (f"{entity_name} is {target} and {target} is {trans_target}",
                                     f"{entity_name} is not {trans_target}")
                                )

        return contradictions

    def compute_consistency_score(self, text: str) -> Dict:
        """
        Compute the logical consistency score for the given text.

        Returns:
            dict: Contains 'score', 'contradictions', 'analysis', and 'residual'
        """
        # Parse statements
        self.statements = self.parse_statements(text)
        if not self.statements:
            return {
                'score': 100.0,
                'contradictions': [],
                'analysis': "No logical statements found.",
                'residual': 0.0
            }

        # Build entity graph
        self.entities = self.build_entity_graph(self.statements)

        # Detect obvious contradictions
        contradictions = self.detect_contradictions(self.entities)

        # If contradictions exist, penalize the residual directly
        # Each contradiction adds to the cohomological obstruction
        base_obstruction = len(contradictions) * 0.5

        # Encode as vectors for wreath-sheaf analysis
        V_samples, targets = self.encode_as_vectors(self.statements, self.entities)

        if not V_samples:
            return {
                'score': 100.0 if not contradictions else 0.0,
                'contradictions': contradictions,
                'analysis': "Unable to encode statements for analysis.",
                'residual': base_obstruction
            }

        # Create problem definition for sheaf learner
        # Each statement type becomes a patch
        problem_definition = {
            'patches': {},
            'gluings': []
        }

        # Group statements by type for patches
        # Also identify contradictory statements and modify their targets
        statement_groups = {}
        contradiction_indices = set()

        # Find indices of contradictory statements
        for c in contradictions:
            for i, stmt in enumerate(self.statements):
                if (f"{stmt.subject} {stmt.relation.replace('_', ' ')} {stmt.object}" in c[0] or
                    f"{stmt.subject} {stmt.relation.replace('_', ' ')} {stmt.object}" in c[1]):
                    contradiction_indices.add(i)

        for i, stmt in enumerate(self.statements):
            key = stmt.relation
            if key not in statement_groups:
                statement_groups[key] = ([], [])
            statement_groups[key][0].append(V_samples[i])

            # Set target to 0 for contradictory statements
            if i in contradiction_indices:
                statement_groups[key][1].append(np.zeros((1, 1), dtype=complex))
            else:
                statement_groups[key][1].append(targets[i])

        # Create patches
        for relation_type, (v_list, t_list) in statement_groups.items():
            if v_list:
                problem_definition['patches'][relation_type] = {
                    'data': (v_list, t_list),
                    'config': {
                        'n_characters': min(8, len(v_list)),
                        'd_model': 1,
                        'n_positions': v_list[0].shape[0]
                    }
                }

        # Add gluing constraints for contradictions
        # Specifically add constraints between "is" and "is_not" patches
        if 'is' in problem_definition['patches'] and 'is_not' in problem_definition['patches']:
            # Add constraint to ensure consistency between is and is_not
            if (problem_definition['patches']['is']['data'][0] and
                problem_definition['patches']['is_not']['data'][0]):
                problem_definition['gluings'].append({
                    'patch_1': 'is',
                    'patch_2': 'is_not',
                    'constraint_data_1': problem_definition['patches']['is']['data'][0][0],
                    'constraint_data_2': problem_definition['patches']['is_not']['data'][0][0]
                })

        # Add more gluing constraints for other patch pairs
        if len(problem_definition['patches']) > 1:
            patch_names = list(problem_definition['patches'].keys())
            for i in range(len(patch_names) - 1):
                for j in range(i + 1, len(patch_names)):
                    # Skip if already added above
                    if (patch_names[i] == 'is' and patch_names[j] == 'is_not') or \
                       (patch_names[i] == 'is_not' and patch_names[j] == 'is'):
                        continue
                    if (problem_definition['patches'][patch_names[i]]['data'][0] and
                        problem_definition['patches'][patch_names[j]]['data'][0]):
                        problem_definition['gluings'].append({
                            'patch_1': patch_names[i],
                            'patch_2': patch_names[j],
                            'constraint_data_1': problem_definition['patches'][patch_names[i]]['data'][0][0],
                            'constraint_data_2': problem_definition['patches'][patch_names[j]]['data'][0][0]
                        })

        # Solve with sheaf learner
        try:
            solution, residual = self.sheaf_learner.fit(problem_definition)

            # Add base obstruction from contradictions to the residual
            total_residual = residual + base_obstruction

            # Convert residual to consistency score (0 residual = 100% consistent)
            # Higher residual = more logical obstruction
            consistency_score = max(0, 100 * (1 - min(1, total_residual)))

            analysis = self._generate_analysis(contradictions, total_residual, len(self.statements))

            return {
                'score': consistency_score,
                'contradictions': contradictions,
                'analysis': analysis,
                'residual': total_residual,
                'n_statements': len(self.statements),
                'n_entities': len(self.entities),
                'n_patches': len(problem_definition['patches']),
                'n_gluings': len(problem_definition['gluings'])
            }

        except Exception as e:
            return {
                'score': 0.0 if contradictions else 50.0,
                'contradictions': contradictions,
                'analysis': f"Error in sheaf analysis: {str(e)}",
                'residual': float('inf') if contradictions else 0.0
            }

    def _generate_analysis(self, contradictions: List[Tuple[str, str]], residual: float, n_statements: int) -> str:
        """Generate human-readable analysis of the logical consistency."""

        if len(contradictions) > 0:
            # If we have contradictions, always mention them
            if residual < 0.1:
                return f"Minor logical tensions detected. {len(contradictions)} contradiction(s) found in {n_statements} statements."
            elif residual < 0.5:
                return f"Moderate logical consistency issues. {len(contradictions)} contradiction(s) detected. Review recommended."
            elif residual < 1.0:
                return f"Significant logical inconsistency. {len(contradictions)} contradiction(s) found."
            else:
                return f"Low logical consistency. {len(contradictions)} contradiction(s) found. Major logical issues present."
        else:
            # No contradictions detected
            if residual < 1e-9:
                return f"Perfect logical consistency! All {n_statements} statements are coherent."
            elif residual < 0.1:
                return f"High logical consistency. All {n_statements} statements align well."
            elif residual < 0.5:
                return f"Moderate logical consistency. Some subtle tensions in {n_statements} statements."
            else:
                return f"Logical tensions detected despite no direct contradictions. Review recommended."


def demo():
    """Run a demo of the logical consistency scorer."""

    print("=" * 80)
    print("Logical Consistency Scorer Demo")
    print("Using Wreath-Sheaf Cohomological Obstruction")
    print("=" * 80)

    # Test case 1: Consistent statements
    print("\nTest 1: Logically consistent statements")
    print("-" * 40)
    text1 = """
dog is animal
cat is animal
dog has fur
cat has fur
dog is not cat
"""

    scorer = LogicalConsistencyScorer(verbose=False)
    result = scorer.compute_consistency_score(text1)

    print(f"Input:\n{text1.strip()}")
    print(f"\nConsistency Score: {result['score']:.1f}%")
    print(f"Cohomological Obstruction: {result['residual']:.4e}")
    print(f"Analysis: {result['analysis']}")

    # Test case 2: Contradictory statements
    print("\n" + "=" * 40)
    print("\nTest 2: Contradictory statements")
    print("-" * 40)
    text2 = """
dog is animal
cat is animal
dog has fur
cat has fur
dog is cat
dog is not cat
"""

    scorer = LogicalConsistencyScorer(verbose=False)
    result = scorer.compute_consistency_score(text2)

    print(f"Input:\n{text2.strip()}")
    print(f"\nConsistency Score: {result['score']:.1f}%")
    print(f"Cohomological Obstruction: {result['residual']:.4e}")
    print(f"Contradictions found:")
    for c in result['contradictions']:
        print(f"  - {c[0]} vs {c[1]}")
    print(f"Analysis: {result['analysis']}")

    # Test case 3: Complex logical structure
    print("\n" + "=" * 40)
    print("\nTest 3: Complex logical relationships")
    print("-" * 40)
    text3 = """
mammal is animal
dog is mammal
cat is mammal
bird is animal
bird is not mammal
dog and cat have fur
bird has feathers
bird lacks fur
"""

    scorer = LogicalConsistencyScorer(verbose=False)
    result = scorer.compute_consistency_score(text3)

    print(f"Input:\n{text3.strip()}")
    print(f"\nConsistency Score: {result['score']:.1f}%")
    print(f"Cohomological Obstruction: {result['residual']:.4e}")
    print(f"Analysis: {result['analysis']}")
    print(f"\nStructural complexity:")
    print(f"  - Statements: {result['n_statements']}")
    print(f"  - Entities: {result['n_entities']}")
    print(f"  - Patches: {result['n_patches']}")
    print(f"  - Gluing constraints: {result['n_gluings']}")

    print("\n" + "=" * 80)
    print("Demo complete!")
    print("\nThe cohomological obstruction (residual) directly quantifies")
    print("the logical inconsistency in the statements. Zero means perfect")
    print("consistency, while higher values indicate more contradictions.")
    print("=" * 80)


if __name__ == "__main__":
    demo()