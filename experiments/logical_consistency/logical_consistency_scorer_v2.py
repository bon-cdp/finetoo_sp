"""
Logical Consistency Scorer V2 - Better Wreath-Sheaf Setup

This version better understands what wreath-sheaf is good at:
- Position-dependent patterns
- Conditional logic with clear structure
- Local patches with global consistency

Instead of trying to directly encode logical statements as vectors,
we set up the problem to match wreath-sheaf's strengths.
"""

import numpy as np
import re
from typing import List, Tuple, Dict, Set, Any
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
class LogicalTriple:
    """A structured logical triple for wreath-sheaf analysis."""
    entity_id: int  # Position in entity list
    relation_id: int  # Type of relation
    target_id: int  # Target entity/property
    truth_value: float  # 1.0 for assertion, -1.0 for negation
    source_stmt: int  # Which statement this came from


class LogicalConsistencyScorerV2:
    """
    Improved scorer that sets up wreath-sheaf problems correctly.
    """

    def __init__(self, verbose=False):
        self.verbose = verbose
        self.statements: List[LogicalStatement] = []
        self.entities: Dict[str, int] = {}  # entity name -> ID
        self.properties: Dict[str, int] = {}  # property name -> ID
        self.relation_types = {
            'is': 0,
            'is_not': 1,
            'has': 2,
            'lacks': 3
        }
        self.sheaf_learner = UnifiedSheafLearner(verbose=verbose)

    def parse_statements(self, text: str) -> List[LogicalStatement]:
        """Parse raw text into logical statements."""
        statements = []
        lines = text.strip().split('\n')

        for idx, line in enumerate(lines):
            line = line.strip()
            if not line or line.startswith('#'):
                continue

            # Pattern matching for different statement types
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

        if self.verbose:
            print(f"\n[DEBUG] Parsed {len(statements)} statements:")
            for stmt in statements:
                print(f"  - {stmt.subject} [{stmt.relation}] {stmt.object}")

        return statements

    def build_knowledge_structure(self, statements: List[LogicalStatement]) -> List[LogicalTriple]:
        """
        Convert statements into structured triples for wreath-sheaf.
        This is where we set up the problem to match what wreath-sheaf is good at.
        """
        triples = []

        # Build entity and property indices
        entity_counter = 0
        property_counter = 0

        for stmt in statements:
            # Register entities
            if stmt.subject not in self.entities:
                self.entities[stmt.subject] = entity_counter
                entity_counter += 1

            if stmt.relation in ['is', 'is_not'] and stmt.object not in self.entities:
                self.entities[stmt.object] = entity_counter
                entity_counter += 1

            # Register properties
            if stmt.relation in ['has', 'lacks'] and stmt.object not in self.properties:
                self.properties[stmt.object] = property_counter
                property_counter += 1

        if self.verbose:
            print(f"\n[DEBUG] Built knowledge structure:")
            print(f"  - Entities: {list(self.entities.keys())}")
            print(f"  - Properties: {list(self.properties.keys())}")

        # Convert statements to triples
        for stmt_idx, stmt in enumerate(statements):
            entity_id = self.entities[stmt.subject]
            relation_id = self.relation_types[stmt.relation]

            if stmt.relation in ['is', 'is_not']:
                target_id = self.entities[stmt.object]
                truth_value = 1.0 if stmt.relation == 'is' else -1.0
            else:  # has/lacks
                target_id = self.properties.get(stmt.object, -1)
                if target_id == -1:
                    continue
                truth_value = 1.0 if stmt.relation == 'has' else -1.0

            triples.append(LogicalTriple(
                entity_id=entity_id,
                relation_id=relation_id,
                target_id=target_id,
                truth_value=truth_value,
                source_stmt=stmt_idx
            ))

        if self.verbose:
            print(f"\n[DEBUG] Created {len(triples)} logical triples")

        return triples

    def detect_contradictions_from_triples(self, triples: List[LogicalTriple]) -> List[Tuple[int, int]]:
        """
        Detect contradictions in the triple structure.
        Returns pairs of triple indices that contradict.
        """
        contradictions = []

        for i, t1 in enumerate(triples):
            for j, t2 in enumerate(triples):
                if i >= j:
                    continue

                # Same entity, same target, opposite truth values
                if (t1.entity_id == t2.entity_id and
                    t1.target_id == t2.target_id and
                    t1.truth_value * t2.truth_value < 0):

                    # Check if relations are compatible for contradiction
                    if (t1.relation_id == 0 and t2.relation_id == 1) or \
                       (t1.relation_id == 1 and t2.relation_id == 0) or \
                       (t1.relation_id == 2 and t2.relation_id == 3) or \
                       (t1.relation_id == 3 and t2.relation_id == 2):
                        contradictions.append((i, j))

        if self.verbose:
            print(f"\n[DEBUG] Found {len(contradictions)} contradictions in triples")

        return contradictions

    def create_wreath_sheaf_problem(self, triples: List[LogicalTriple],
                                   contradictions: List[Tuple[int, int]]) -> Dict:
        """
        Create a wreath-sheaf problem that plays to its strengths:
        - Each relation type becomes a patch
        - Contradictions create gluing constraints
        - Position encodes entity ID (what wreath products are good at!)
        """

        # Group triples by relation type (these become patches)
        patches_data = {}
        for triple in triples:
            rel_name = list(self.relation_types.keys())[list(self.relation_types.values()).index(triple.relation_id)]
            if rel_name not in patches_data:
                patches_data[rel_name] = []
            patches_data[rel_name].append(triple)

        if self.verbose:
            print(f"\n[DEBUG] Creating wreath-sheaf problem:")
            for patch_name, patch_triples in patches_data.items():
                print(f"  - Patch '{patch_name}': {len(patch_triples)} triples")

        # Create the problem definition
        problem_definition = {
            'patches': {},
            'gluings': []
        }

        # Convert each patch's triples to vectors
        n_entities = len(self.entities)
        n_properties = len(self.properties)
        vector_dim = max(n_entities, n_properties, 4)  # At least 4 dimensions

        for patch_name, patch_triples in patches_data.items():
            V_samples = []
            targets = []

            for triple in patch_triples:
                # Create position-dependent vector (what wreath is good at!)
                v = np.zeros((vector_dim, 1), dtype=complex)

                # Position encodes entity (wreath product strength)
                v[triple.entity_id % vector_dim, 0] = 1.0

                # Encode relation and target
                if triple.relation_id < 2:  # is/is_not
                    v[(triple.target_id + 1) % vector_dim, 0] = 0.5 * triple.truth_value
                else:  # has/lacks
                    v[(triple.target_id + 2) % vector_dim, 0] = 0.5 * triple.truth_value

                V_samples.append(v)

                # Target: 1 if no contradiction, 0 if contradiction
                is_contradicted = any(
                    (i == patch_triples.index(triple) or j == patch_triples.index(triple))
                    for i, j in contradictions
                    if i < len(triples) and j < len(triples)
                )
                targets.append(np.array([[0.0 if is_contradicted else 1.0]], dtype=complex))

            if V_samples:
                problem_definition['patches'][patch_name] = {
                    'data': (V_samples, targets),
                    'config': {
                        'n_characters': min(8, len(V_samples)),
                        'd_model': 1,
                        'n_positions': vector_dim
                    }
                }

        # Add gluing constraints for contradictions
        # This is where sheaf theory shines!
        if 'is' in problem_definition['patches'] and 'is_not' in problem_definition['patches']:
            if problem_definition['patches']['is']['data'][0] and \
               problem_definition['patches']['is_not']['data'][0]:
                problem_definition['gluings'].append({
                    'patch_1': 'is',
                    'patch_2': 'is_not',
                    'constraint_data_1': problem_definition['patches']['is']['data'][0][0],
                    'constraint_data_2': problem_definition['patches']['is_not']['data'][0][0]
                })

        if 'has' in problem_definition['patches'] and 'lacks' in problem_definition['patches']:
            if problem_definition['patches']['has']['data'][0] and \
               problem_definition['patches']['lacks']['data'][0]:
                problem_definition['gluings'].append({
                    'patch_1': 'has',
                    'patch_2': 'lacks',
                    'constraint_data_1': problem_definition['patches']['has']['data'][0][0],
                    'constraint_data_2': problem_definition['patches']['lacks']['data'][0][0]
                })

        if self.verbose:
            print(f"  - Total gluings: {len(problem_definition['gluings'])}")

        return problem_definition

    def compute_consistency_score(self, text: str) -> Dict:
        """
        Compute the logical consistency score using improved wreath-sheaf setup.
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

        # Build structured knowledge representation
        triples = self.build_knowledge_structure(self.statements)

        # Detect contradictions
        triple_contradictions = self.detect_contradictions_from_triples(triples)

        # Convert back to statement contradictions for display
        statement_contradictions = []
        for i, j in triple_contradictions:
            if i < len(triples) and j < len(triples):
                stmt1_idx = triples[i].source_stmt
                stmt2_idx = triples[j].source_stmt
                if stmt1_idx < len(self.statements) and stmt2_idx < len(self.statements):
                    stmt1 = self.statements[int(stmt1_idx)]
                    stmt2 = self.statements[int(stmt2_idx)]
                    statement_contradictions.append((
                        f"{stmt1.subject} {stmt1.relation.replace('_', ' ')} {stmt1.object}",
                        f"{stmt2.subject} {stmt2.relation.replace('_', ' ')} {stmt2.object}"
                    ))

        # Create wreath-sheaf problem
        problem_definition = self.create_wreath_sheaf_problem(triples, triple_contradictions)

        if self.verbose:
            print(f"\n[DEBUG] Problem has {len(problem_definition['patches'])} patches")

        # Base obstruction from contradictions
        base_obstruction = len(statement_contradictions) * 0.5

        # Solve with sheaf learner
        try:
            if problem_definition['patches']:
                solution, residual = self.sheaf_learner.fit(problem_definition)

                if self.verbose:
                    print(f"\n[DEBUG] Sheaf solver results:")
                    print(f"  - Raw residual: {residual:.4e}")
                    print(f"  - Base obstruction: {base_obstruction:.4e}")
            else:
                residual = 0.0

            # Total obstruction
            total_residual = residual + base_obstruction

            # Convert to score
            consistency_score = max(0, 100 * (1 - min(1, total_residual)))

            if self.verbose:
                print(f"  - Total residual: {total_residual:.4e}")
                print(f"  - Consistency score: {consistency_score:.1f}%")

            # Generate analysis
            if len(statement_contradictions) > 0:
                if total_residual < 0.5:
                    analysis = f"Minor contradictions found ({len(statement_contradictions)} issues)."
                elif total_residual < 1.0:
                    analysis = f"Moderate logical inconsistency ({len(statement_contradictions)} contradictions)."
                else:
                    analysis = f"Significant logical issues ({len(statement_contradictions)} contradictions)."
            else:
                if total_residual < 1e-9:
                    analysis = f"Perfect logical consistency! All {len(self.statements)} statements coherent."
                else:
                    analysis = f"Good logical consistency with minor tensions."

            return {
                'score': consistency_score,
                'contradictions': statement_contradictions,
                'analysis': analysis,
                'residual': total_residual,
                'n_statements': len(self.statements),
                'n_entities': len(self.entities),
                'n_properties': len(self.properties),
                'n_triples': len(triples),
                'n_patches': len(problem_definition['patches']),
                'n_gluings': len(problem_definition['gluings'])
            }

        except Exception as e:
            if self.verbose:
                print(f"\n[DEBUG] Error in sheaf solver: {e}")

            return {
                'score': 0.0 if statement_contradictions else 50.0,
                'contradictions': statement_contradictions,
                'analysis': f"Error in analysis: {str(e)}",
                'residual': float('inf') if statement_contradictions else 0.0
            }


def demo():
    """Run demo with debugging enabled."""

    print("=" * 80)
    print("Logical Consistency Scorer V2 - Improved Wreath-Sheaf Setup")
    print("=" * 80)

    # Test with debugging ON
    scorer = LogicalConsistencyScorerV2(verbose=True)

    # Test case 1: Simple contradiction
    print("\n" + "=" * 60)
    print("TEST 1: Direct Contradiction")
    print("=" * 60)

    text1 = """
dog is animal
cat is animal
dog is cat
dog is not cat
"""

    result = scorer.compute_consistency_score(text1)

    print(f"\n[RESULTS]")
    print(f"  Score: {result['score']:.1f}%")
    print(f"  Residual: {result['residual']:.4e}")
    print(f"  Analysis: {result['analysis']}")
    if result['contradictions']:
        print(f"  Contradictions:")
        for c in result['contradictions']:
            print(f"    - '{c[0]}' vs '{c[1]}'")

    # Test case 2: Complex but consistent
    print("\n" + "=" * 60)
    print("TEST 2: Complex Consistent Statements")
    print("=" * 60)

    text2 = """
mammal is animal
dog is mammal
cat is mammal
dog has fur
cat has fur
dog is not cat
"""

    scorer2 = LogicalConsistencyScorerV2(verbose=True)
    result2 = scorer2.compute_consistency_score(text2)

    print(f"\n[RESULTS]")
    print(f"  Score: {result2['score']:.1f}%")
    print(f"  Residual: {result2['residual']:.4e}")
    print(f"  Analysis: {result2['analysis']}")

    # Test case 3: Property contradiction
    print("\n" + "=" * 60)
    print("TEST 3: Property Contradiction")
    print("=" * 60)

    text3 = """
bird has wings
bird lacks wings
"""

    scorer3 = LogicalConsistencyScorerV2(verbose=True)
    result3 = scorer3.compute_consistency_score(text3)

    print(f"\n[RESULTS]")
    print(f"  Score: {result3['score']:.1f}%")
    print(f"  Residual: {result3['residual']:.4e}")
    print(f"  Analysis: {result3['analysis']}")
    if result3['contradictions']:
        print(f"  Contradictions:")
        for c in result3['contradictions']:
            print(f"    - '{c[0]}' vs '{c[1]}'")

    print("\n" + "=" * 80)
    print("Demo complete!")
    print("=" * 80)


if __name__ == "__main__":
    demo()