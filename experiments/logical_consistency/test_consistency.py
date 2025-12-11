#!/usr/bin/env python3
"""
Test Suite for Logical Consistency Scorer

Validates that the wreath-sheaf cohomological obstruction correctly
identifies logical consistency and contradictions.
"""

import unittest
import numpy as np
from logical_consistency_scorer import LogicalConsistencyScorer


class TestLogicalConsistency(unittest.TestCase):
    """Test cases for logical consistency scoring."""

    def setUp(self):
        """Set up test fixtures."""
        self.scorer = LogicalConsistencyScorer(verbose=False)

    def test_perfect_consistency(self):
        """Test perfectly consistent statements."""
        text = """
        dog is animal
        cat is animal
        bird is animal
        dog has fur
        cat has fur
        bird has feathers
        """

        result = self.scorer.compute_consistency_score(text)

        self.assertGreater(result['score'], 90.0)
        self.assertEqual(len(result['contradictions']), 0)
        self.assertLess(result['residual'], 0.1)

    def test_direct_contradiction(self):
        """Test direct contradictions."""
        text = """
        dog is animal
        dog is not animal
        """

        result = self.scorer.compute_consistency_score(text)

        self.assertLess(result['score'], 50.0)
        self.assertGreater(len(result['contradictions']), 0)
        # Check that the contradiction is detected
        found_contradiction = False
        for c in result['contradictions']:
            if 'dog is animal' in c[0] and 'dog is not animal' in c[1]:
                found_contradiction = True
        self.assertTrue(found_contradiction)

    def test_property_contradiction(self):
        """Test property contradictions (has/lacks)."""
        text = """
        bird has wings
        bird lacks wings
        """

        result = self.scorer.compute_consistency_score(text)

        self.assertLess(result['score'], 70.0)
        self.assertGreater(len(result['contradictions']), 0)

    def test_transitive_consistency(self):
        """Test transitive relationships."""
        text = """
        dog is mammal
        mammal is animal
        dog is animal
        """

        result = self.scorer.compute_consistency_score(text)

        self.assertGreater(result['score'], 80.0)
        self.assertEqual(len(result['contradictions']), 0)

    def test_transitive_violation(self):
        """Test violations of transitivity."""
        text = """
        dog is mammal
        mammal is animal
        dog is not animal
        """

        result = self.scorer.compute_consistency_score(text)

        self.assertLess(result['score'], 60.0)
        self.assertGreater(len(result['contradictions']), 0)

    def test_complex_scenario(self):
        """Test complex logical structure."""
        text = """
        mammal is animal
        bird is animal
        fish is animal
        dog is mammal
        cat is mammal
        sparrow is bird
        salmon is fish
        dog has fur
        cat has fur
        sparrow has feathers
        salmon has scales
        mammal is not bird
        bird is not fish
        fish is not mammal
        """

        result = self.scorer.compute_consistency_score(text)

        # Should be highly consistent
        self.assertGreater(result['score'], 75.0)
        # No direct contradictions
        self.assertEqual(len(result['contradictions']), 0)

    def test_mixed_consistency(self):
        """Test mixture of consistent and inconsistent statements."""
        text = """
        dog is animal
        cat is animal
        dog has fur
        cat has fur
        dog is cat
        dog is not cat
        bird is animal
        bird has feathers
        """

        result = self.scorer.compute_consistency_score(text)

        # Should detect the dog/cat contradiction
        self.assertLess(result['score'], 80.0)
        self.assertGreater(len(result['contradictions']), 0)

        # But should still recognize partial consistency
        self.assertGreater(result['score'], 20.0)

    def test_empty_input(self):
        """Test empty input handling."""
        text = ""

        result = self.scorer.compute_consistency_score(text)

        self.assertEqual(result['score'], 100.0)
        self.assertEqual(len(result['contradictions']), 0)
        self.assertEqual(result['residual'], 0.0)

    def test_no_logical_statements(self):
        """Test input with no parseable logical statements."""
        text = """
        This is just regular text.
        No logical statements here!
        """

        result = self.scorer.compute_consistency_score(text)

        self.assertEqual(result['score'], 100.0)
        self.assertEqual(len(result['contradictions']), 0)

    def test_statement_parsing(self):
        """Test various statement patterns."""
        statements = self.scorer.parse_statements("""
        dog is animal
        cat is not dog
        bird has wings
        fish lacks wings
        dog and cat have fur
        """)

        self.assertEqual(len(statements), 6)  # dog and cat expands to 2

        # Check specific parsing
        self.assertEqual(statements[0].subject, "dog")
        self.assertEqual(statements[0].relation, "is")
        self.assertEqual(statements[0].object, "animal")

        self.assertEqual(statements[1].subject, "cat")
        self.assertEqual(statements[1].relation, "is_not")
        self.assertEqual(statements[1].object, "dog")

        self.assertEqual(statements[2].subject, "bird")
        self.assertEqual(statements[2].relation, "has")
        self.assertEqual(statements[2].object, "wings")

    def test_entity_graph_building(self):
        """Test entity graph construction."""
        statements = self.scorer.parse_statements("""
        dog is animal
        dog has fur
        dog is not cat
        """)

        entities = self.scorer.build_entity_graph(statements)

        self.assertIn("dog", entities)
        self.assertIn("animal", entities)
        self.assertIn("cat", entities)

        # Check relationships
        self.assertIn("is", entities["dog"].relations)
        self.assertIn("animal", entities["dog"].relations["is"])
        self.assertIn("fur", entities["dog"].properties)

    def test_vector_encoding(self):
        """Test vector encoding of statements."""
        statements = self.scorer.parse_statements("dog is animal")
        entities = self.scorer.build_entity_graph(statements)
        V_samples, targets = self.scorer.encode_as_vectors(statements, entities)

        self.assertEqual(len(V_samples), 1)
        self.assertEqual(len(targets), 1)
        self.assertIsInstance(V_samples[0], np.ndarray)
        self.assertGreater(V_samples[0].shape[0], 0)


class TestConsistencyPatterns(unittest.TestCase):
    """Test specific logical consistency patterns."""

    def setUp(self):
        """Set up test fixtures."""
        self.scorer = LogicalConsistencyScorer(verbose=False)

    def test_symmetry_violation(self):
        """Test symmetry violations (if A is B, then B should be A for equality)."""
        text = """
        dog is pet
        pet is not dog
        """

        # Note: This isn't necessarily a contradiction in formal logic
        # (subset vs equality), but it creates tension
        result = self.scorer.compute_consistency_score(text)

        # Should have moderate consistency (not a direct contradiction)
        self.assertGreater(result['score'], 40.0)
        self.assertLess(result['score'], 90.0)

    def test_mutual_exclusivity(self):
        """Test mutual exclusivity patterns."""
        text = """
        animal is living
        plant is living
        rock is not living
        animal is not plant
        plant is not rock
        rock is not animal
        """

        result = self.scorer.compute_consistency_score(text)

        # Should be highly consistent (proper categorization)
        self.assertGreater(result['score'], 80.0)
        self.assertEqual(len(result['contradictions']), 0)

    def test_hierarchical_consistency(self):
        """Test hierarchical relationships."""
        text = """
        poodle is dog
        dog is mammal
        mammal is animal
        animal is organism
        poodle is organism
        """

        result = self.scorer.compute_consistency_score(text)

        # Should be perfectly consistent
        self.assertGreater(result['score'], 90.0)
        self.assertEqual(len(result['contradictions']), 0)

    def test_circular_reference(self):
        """Test circular references (A is B, B is C, C is A)."""
        text = """
        rock is scissors
        scissors is paper
        paper is rock
        """

        result = self.scorer.compute_consistency_score(text)

        # Circular references create logical tension
        # but aren't necessarily contradictions
        self.assertGreater(result['score'], 50.0)


class TestRealWorldScenarios(unittest.TestCase):
    """Test real-world knowledge scenarios."""

    def setUp(self):
        """Set up test fixtures."""
        self.scorer = LogicalConsistencyScorer(verbose=False)

    def test_scientific_facts(self):
        """Test scientific facts consistency."""
        text = """
        water is liquid
        ice is solid
        steam is gas
        ice is water
        steam is water
        liquid is not solid
        solid is not gas
        gas is not liquid
        """

        result = self.scorer.compute_consistency_score(text)

        # This creates some tension (ice is water, but ice is solid and water is liquid)
        # The system should detect this complexity
        self.assertGreater(result['score'], 30.0)
        self.assertLess(result['score'], 90.0)

    def test_taxonomic_knowledge(self):
        """Test biological taxonomy."""
        text = """
        human is primate
        primate is mammal
        mammal is vertebrate
        vertebrate is animal
        human has intelligence
        human has language
        primate has intelligence
        mammal has hair
        vertebrate has backbone
        """

        result = self.scorer.compute_consistency_score(text)

        # Should be highly consistent
        self.assertGreater(result['score'], 80.0)

    def test_contradictory_beliefs(self):
        """Test contradictory belief systems."""
        text = """
        technology is beneficial
        technology is harmful
        progress is good
        progress is dangerous
        AI is tool
        AI is threat
        """

        result = self.scorer.compute_consistency_score(text)

        # Should detect the contradictions
        self.assertLess(result['score'], 60.0)
        self.assertGreater(len(result['contradictions']), 0)


def run_tests():
    """Run all tests and print summary."""
    print("=" * 80)
    print(" Running Logical Consistency Test Suite ".center(80))
    print("=" * 80)

    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    # Add all test classes
    suite.addTests(loader.loadTestsFromTestCase(TestLogicalConsistency))
    suite.addTests(loader.loadTestsFromTestCase(TestConsistencyPatterns))
    suite.addTests(loader.loadTestsFromTestCase(TestRealWorldScenarios))

    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    # Print summary
    print("\n" + "=" * 80)
    if result.wasSuccessful():
        print(" ✓ ALL TESTS PASSED ".center(80))
    else:
        print(f" ✗ {len(result.failures)} FAILURES, {len(result.errors)} ERRORS ".center(80))
    print("=" * 80)

    return result.wasSuccessful()


if __name__ == "__main__":
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)