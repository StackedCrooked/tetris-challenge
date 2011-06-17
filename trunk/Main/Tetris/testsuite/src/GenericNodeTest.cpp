#include "GenericNodeTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/Playground/GenericNode.h"
#include "Futile/Allocator.h"
#include "Futile/Assert.h"
#include <iostream>


GenericNodeTest::GenericNodeTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


GenericNodeTest::~GenericNodeTest()
{
}


void GenericNodeTest::testNode()
{
    typedef GenericNode<int> Node;
    Node node;

    node.insert(new Node(1));
    Assert(!HasCycles(node));

    node.insert(new Node(2));
    Assert(!HasCycles(node));

    node.insert(new Node(3));
    Assert(!HasCycles(node));

    node.insert(new Node(4));
    Assert(!HasCycles(node));

    // Insert a cycle
    node.insert(new Node(3));
    Assert(HasCycles(node));

    std::cout << "NodeTest passed succesfully.\n";
}


void GenericNodeTest::setUp()
{
}


void GenericNodeTest::tearDown()
{
}


CppUnit::Test * GenericNodeTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("NodeTest"));
    CppUnit_addTest(suite, GenericNodeTest, testNode);
    return suite;
}
