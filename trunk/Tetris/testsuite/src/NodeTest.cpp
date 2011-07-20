#include "NodeTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Futile/Node.h"
#include "Futile/Assert.h"
#include <iostream>


using Futile::GenericNode;
using Futile::HasCycles;


NodeTest::NodeTest(const std::string & inName):
    CppUnit::TestCase(inName)
{
}


NodeTest::~NodeTest()
{
}


void NodeTest::testNode()
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


void NodeTest::setUp()
{
}


void NodeTest::tearDown()
{
}


CppUnit::Test * NodeTest::suite()
{
    CppUnit::TestSuite * suite(new CppUnit::TestSuite("NodeTest"));
    CppUnit_addTest(suite, NodeTest, testNode);
    return suite;
}
