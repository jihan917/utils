/**
 * tsort - topological sort
 *
 * (C) Copyright 2010, Ji Han (jihan917<at>yahoo<dot>com).
 * free to distribute under the GPL license.
 *
 * SYNOPSIS: tsort [file]
 *
 * see http://www.opengroup.org/onlinepubs/009695399/utilities/tsort.html
 * for the specification.
 */


#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>


struct Graph
{
    struct Node
    {
        typedef std::string Key;
        typedef std::set<Key> Neighbors;

        int indegree_;
        Neighbors neighbors_;

        Node()
            : indegree_(0)
        {
        }

    };

    typedef std::pair<Node::Key, Node::Key> Edge;
    typedef std::map<Node::Key, Node> Nodes;

    Nodes nodes_;

    Graph() {}
    Graph(const Graph& other)
        : nodes_ (other.nodes_)
    {
    }

    void clear()
    {
        nodes_.clear();
    }

    const bool has_node(Node::Key key) const
    {
        return (nodes_.find(key) != nodes_.end());
    }

    void new_node(Node::Key key)
    {
        if (!has_node(key))
        {
            nodes_[key] = Node();
        }
    }

    void remove_node(Node::Key key)
    {
        if (nodes_.find(key) == nodes_.end())
        {
            return;
        }

        for (Node::Neighbors::iterator it = nodes_[key].neighbors_.begin();
             it != nodes_[key].neighbors_.end();
             ++it)
        {
            nodes_[*it].neighbors_.erase(key);
            --nodes_[*it].indegree_;
        }
        nodes_.erase(key);
    }

    const bool has_edge(Node::Key from, Node::Key to) const
    {
        if (!has_node(from))
        {
            return false;
        }

        const Node::Neighbors& neighbors =
            nodes_.find(from)->second.neighbors_;
        return (neighbors.find(to) != neighbors.end());
    }

    void new_edge(Node::Key from, Node::Key to)
    {
        if (!has_edge(from, to))
        {
            nodes_[from].neighbors_.insert(to);
            ++nodes_[to].indegree_;
        }
    }

    void remove_edge(Node::Key from, Node::Key to)
    {
        if (has_edge(from, to))
        {
            nodes_[from].neighbors_.erase(to);
            --nodes_[to].indegree_;
        }
    }

    void new_edge(Edge e)
    {
        new_edge(e.first, e.second);
    }

    void remove_edge(Edge e)
    {
        remove_edge(e.first, e.second);
    }
};


const bool tsort(Graph& graph, std::list<Graph::Node::Key>& order)
{
    std::list<Graph::Node::Key> openset;
    for (Graph::Nodes::iterator it = graph.nodes_.begin();
         it != graph.nodes_.end();
         ++it)
    {
        if (it->second.indegree_ == 0)
        {
            openset.push_back(it->first);
        }
    }

    order.clear();
    while (!openset.empty())
    {
        order.push_back(openset.front());
        Graph::Node::Neighbors& neighbors =
            graph.nodes_[openset.front()].neighbors_;
        while (!neighbors.empty())
        {
            Graph::Node::Key key(*neighbors.begin());
            graph.remove_edge(openset.front(), key);
            if (graph.nodes_[key].indegree_ == 0)
                openset.push_back(key);
        }
        openset.pop_front();
    }

    for (Graph::Nodes::iterator it = graph.nodes_.begin();
         it != graph.nodes_.end();
         ++it)
    {
        if (!it->second.neighbors_.empty())
        {
            return false;
        }
    }

    return true;
}


#define Q(x) #x
#define QQ(x) Q(x)

int main(int argc, char **argv)
{
    if (argv[1])
    {
        if (!freopen(argv[1], "r", stdin))
        {
            exit(EXIT_FAILURE);
        }
    }

    Graph graph;
    std::list<Graph::Node::Key> order;

    char left[BUFSIZ], right[BUFSIZ];
    while (fscanf(stdin,
                  "%" QQ(BUFSIZ) "s"
                  "%" QQ(BUFSIZ) "s",
                  left,
                  right) == 2)
    {
        std::string a(left), b(right);
        if (a == b)
        {
            graph.new_node(a);
        }
        else
        {
            graph.new_edge(a, b);
        }
    }

    if (tsort(graph, order))
    {
        std::copy(order.begin(),
                  order.end(),
                  std::ostream_iterator<std::string>(std::cout, "\n"));
    }

    return 0;
}

