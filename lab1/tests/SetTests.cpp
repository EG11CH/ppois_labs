#include "TestFramework.h"
#include "lab1/Set.h"

#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

using lab1::Set;
using lab1::SetElement;

TEST_CASE("Set distinguishes an empty set from a set containing the empty set", "[set]") {
    const Set empty;
    const Set containingEmpty{SetElement(empty)};
    REQUIRE(empty.empty());
    REQUIRE(empty.size() == 0);
    REQUIRE_FALSE(containingEmpty.empty());
    REQUIRE(containingEmpty.size() == 1);
    REQUIRE(containingEmpty[SetElement(empty)]);
    REQUIRE(empty != containingEmpty);
}

TEST_CASE("Set deduplicates atoms and returns meaningful add remove flags", "[set]") {
    Set set;
    const SetElement atom("a");
    REQUIRE(set.add(atom));
    REQUIRE_FALSE(set.add(atom));
    REQUIRE(set[atom]);
    REQUIRE(set.size() == 1);
    REQUIRE_FALSE(set.remove(SetElement("missing")));
    REQUIRE(set.remove(atom));
    REQUIRE_FALSE(set[atom]);
    REQUIRE(set.empty());
}

TEST_CASE("Set initializer list deduplicates nested sets irrespective of order", "[set]") {
    const Set first = Set::fromString("{a, b}");
    const Set second = Set::fromString("{b, a, a}");
    const Set set{SetElement(first), SetElement(second), SetElement("a")};
    REQUIRE(set.size() == 2);
    REQUIRE(set[SetElement(second)]);
    REQUIRE(set[SetElement("a")]);
}

TEST_CASE("Set parses the complete assignment example", "[set]") {
    const Set set = Set::fromString("{a, b, c, {a, b}, {}, {a, {c}}}");
    REQUIRE(set.size() == 6);
    REQUIRE(set[SetElement(Set::fromString("{b, a}"))]);
    REQUIRE(set[SetElement(Set{})]);
    REQUIRE(set[SetElement(Set::fromString("{{c}, a}"))]);
    REQUIRE_FALSE(set[SetElement("d")]);
}

TEST_CASE("Set equality ignores all nested orders but distinguishes types", "[set]") {
    const Set first = Set::fromString("{a, {b, c}, {}} ");
    const Set second = Set::fromString("{ {}, {c, b}, a, a}");
    REQUIRE(first == second);
    REQUIRE(first != Set::fromString("{a, {b, d}, {}}"));
    REQUIRE(Set::fromString("{\"{a}\"}") != Set::fromString("{{a}}"));
}

TEST_CASE("Set serialization is deterministic and quotes special characters", "[set]") {
    const Set set = Set::fromString(
        R"({a, "two words", "a,b", "quote\"inside", "slash\\inside", "{a}", русский})");
    const std::string text = set.toString();
    REQUIRE(Set::fromString(text) == set);
    REQUIRE(text.find("\"two words\"") != std::string::npos);
    REQUIRE(Set::fromString("{b, a}").toString() == "{a, b}");
    REQUIRE(Set::fromString("{}").toString() == "{}");
}

TEST_CASE("Set rejects malformed input", "[set]") {
    const std::vector<std::string> invalid = {"",
                                              "a",
                                              "{",
                                              "{a",
                                              "{a,}",
                                              "{,a}",
                                              "{a b}",
                                              "{a,,b}",
                                              "{} extra",
                                              "{\"\"}",
                                              "{\"unterminated}",
                                              "{a{b}}",
                                              "{a\\b}",
                                              "{a\"b}",
                                              "{{a}, {b}"};
    for (const auto& text : invalid) {
        INFO(text);
        REQUIRE_THROWS_AS(Set::fromString(text), std::invalid_argument);
    }
}

TEST_CASE("Set validates parser nesting depth at the boundary", "[set]") {
    const std::string valid =
        std::string(Set::maxParseDepth, '{') + std::string(Set::maxParseDepth, '}');
    REQUIRE_NOTHROW(Set::fromString(valid));
    const std::string tooDeep = "{" + valid + "}";
    REQUIRE_THROWS_AS(Set::fromString(tooDeep), std::invalid_argument);
}

TEST_CASE("Set stream extraction reads successive objects", "[set]") {
    std::istringstream input(" {a, {b}} {} ");
    Set first;
    Set second;
    input >> first >> second;
    REQUIRE_FALSE(input.fail());
    REQUIRE(first == Set::fromString("{a, {b}}"));
    REQUIRE(second.empty());
}

TEST_CASE("Set invalid stream extraction preserves the existing object", "[set]") {
    Set set = Set::fromString("{keep}");
    std::istringstream input("{a,}");
    input >> set;
    REQUIRE(input.fail());
    REQUIRE(set == Set::fromString("{keep}"));
}

TEST_CASE("Set union includes each element once without mutating operands", "[set]") {
    Set left = Set::fromString("{a, {b}, {}}");
    const Set right = Set::fromString("{c, {b}}");
    const Set result = left + right;
    REQUIRE(result == Set::fromString("{a, c, {b}, {}}"));
    REQUIRE(left.size() == 3);
    REQUIRE(right.size() == 2);
    REQUIRE(&(left += right) == &left);
    REQUIRE(left == result);
    left += left;
    REQUIRE(left == result);
}

TEST_CASE("Set intersection preserves common atoms and nested sets", "[set]") {
    Set left = Set::fromString("{a, {b, c}, {}}");
    const Set right = Set::fromString("{a, {c, b}, z}");
    const Set result = left * right;
    REQUIRE(result == Set::fromString("{a, {b, c}}"));
    REQUIRE(left.size() == 3);
    REQUIRE(&(left *= right) == &left);
    REQUIRE(left == result);
    left *= left;
    REQUIRE(left == result);
    REQUIRE((left * Set{}).empty());
}

TEST_CASE("Set difference is directional and self subtraction is empty", "[set]") {
    Set left = Set::fromString("{a, b, {c}}");
    const Set right = Set::fromString("{b, d}");
    const Set result = left - right;
    REQUIRE(result == Set::fromString("{a, {c}}"));
    REQUIRE((right - left) == Set::fromString("{d}"));
    REQUIRE(&(left -= right) == &left);
    REQUIRE(left == result);
    left -= left;
    REQUIRE(left.empty());
}

TEST_CASE("Set operations have correct empty set identities", "[set]") {
    const Set nonempty = Set::fromString("{a}");
    const Set empty;
    REQUIRE((nonempty + empty) == nonempty);
    REQUIRE((empty + nonempty) == nonempty);
    REQUIRE((nonempty - empty) == nonempty);
    REQUIRE((empty - nonempty).empty());
    REQUIRE((empty * nonempty).empty());
}

TEST_CASE("Set boolean contains every subset of two atoms", "[set]") {
    const Set original = Set::fromString("{a, b}");
    const Set subsets = original.powerSet();
    REQUIRE(subsets.size() == 4);
    REQUIRE(subsets == Set::fromString("{{}, {a}, {b}, {a, b}}"));
    REQUIRE(original == Set::fromString("{a, b}"));
}

TEST_CASE("Set boolean treats a nested set as one whole element", "[set]") {
    const Set original = Set::fromString("{a, {b, c}}");
    const Set subsets = original.powerSet();
    REQUIRE(subsets.size() == 4);
    REQUIRE(subsets == Set::fromString("{{}, {a}, {{b, c}}, {a, {b, c}}}"));
    REQUIRE(Set{}.powerSet() == Set::fromString("{{}}"));
}

TEST_CASE("Set boolean limit is configurable and checked before allocation", "[set]") {
    const Set original = Set::fromString("{a, b, c}");
    REQUIRE(original.powerSet(8).size() == 8);
    REQUIRE_THROWS_AS(original.powerSet(7), std::length_error);
    REQUIRE_THROWS_AS(Set{}.powerSet(0), std::length_error);
    Set tooLarge;
    constexpr std::size_t atomCount = 13;
    for (std::size_t index = 0; index < atomCount; ++index) {
        tooLarge.add(SetElement(std::to_string(index)));
    }
    REQUIRE_THROWS_AS(tooLarge.powerSet(), std::length_error);
}

TEST_CASE("Set boolean count overflow is caught for the maximum size_t budget", "[set]") {
    Set tooLarge;
    const auto bits = static_cast<std::size_t>(std::numeric_limits<std::size_t>::digits);
    for (std::size_t index = 0; index < bits; ++index) {
        tooLarge.add(SetElement(std::to_string(index)));
    }
    REQUIRE_THROWS_AS(tooLarge.powerSet(std::numeric_limits<std::size_t>::max()),
                      std::length_error);
}

TEST_CASE("Set copies and assignments are independent including nested values", "[set]") {
    Set nested = Set::fromString("{a}");
    Set original{SetElement(nested)};
    nested.add(SetElement("changed"));
    REQUIRE(original == Set::fromString("{{a}}"));
    Set copy(original);
    Set assigned = Set::fromString("{old}");
    REQUIRE(&(assigned = original) == &assigned);
    copy.add(SetElement("b"));
    assigned.remove(SetElement(Set::fromString("{a}")));
    original = original;
    REQUIRE(original == Set::fromString("{{a}}"));
    REQUIRE(copy.size() == 2);
    REQUIRE(assigned.empty());
}

TEST_CASE("Set moves and swaps retain value semantics", "[set]") {
    Set source = Set::fromString("{{a}, b}");
    Set moved(std::move(source));
    Set target;
    target = std::move(moved);
    REQUIRE(target == Set::fromString("{{a}, b}"));
    Set other = Set::fromString("{c}");
    target.swap(other);
    REQUIRE(target == Set::fromString("{c}"));
    REQUIRE(other == Set::fromString("{{a}, b}"));
}

TEST_CASE("SetElement validates its type and distinguishes atom from nested set", "[set-element]") {
    const SetElement atom("a");
    const SetElement nested(Set::fromString("{a}"));
    REQUIRE_FALSE(atom.isSet());
    REQUIRE(atom.asAtom() == "a");
    REQUIRE(nested.isSet());
    REQUIRE(nested.asSet() == Set::fromString("{a}"));
    REQUIRE_THROWS_AS(atom.asSet(), std::logic_error);
    REQUIRE_THROWS_AS(nested.asAtom(), std::logic_error);
    REQUIRE_THROWS_AS(SetElement(""), std::invalid_argument);
    REQUIRE(atom != nested);
}

TEST_CASE("SetElement supports deep copy assignment self assignment and moves", "[set-element]") {
    const SetElement nested(Set::fromString("{a}"));
    SetElement assigned("old");
    assigned = nested;
    assigned = assigned;
    REQUIRE(assigned == nested);
    SetElement copied(assigned);
    SetElement moved(std::move(copied));
    SetElement moveAssigned("other");
    moveAssigned = std::move(moved);
    REQUIRE(moveAssigned == nested);
    REQUIRE(moveAssigned.toString() == "{a}");
}

TEST_CASE("SetElement text parsing handles atoms and nested sets", "[set-element]") {
    REQUIRE(SetElement::fromString("a").asAtom() == "a");
    REQUIRE(Set::elementFromString(" \"two words\" ").asAtom() == "two words");
    REQUIRE(Set::elementFromString("{a, b}").asSet() == Set::fromString("{b, a}"));
    REQUIRE_THROWS_AS(Set::elementFromString("a b"), std::invalid_argument);
    REQUIRE_THROWS_AS(Set::elementFromString(""), std::invalid_argument);
}
