//Kevin Carlisle
//SID #82682616
//UID #848520

#ifndef HASH_GRAPH_HPP_
#define HASH_GRAPH_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "iterator.hpp"
#include "pair.hpp"
#include "heap_priority_queue.hpp"
#include "hash_set.hpp"
#include "hash_map.hpp"


namespace ics {

template<class T>
class HashGraph {

  //Forward declaration: see the return type of all_nodes
  private:
    class LocalInfo;

  public:
    //Commands
    HashGraph();
    HashGraph(HashGraph& g);
    virtual ~HashGraph() {}
    void add_node    (std::string node_name);
    void add_edge    (std::string origin, std::string destination, T value);
    void remove_node (std::string node_name);
    void remove_edge (std::string origin, std::string destination);
    void clear       ();
    void load (std::ifstream& in_file,  std::string separator = ";");
    void store(std::ofstream& out_file, std::string separator = ";");

    //Queries
    bool empty      () const;
    int  node_count () const;
    int  edge_count () const;
    bool has_node   (std::string node_name) const;
    bool has_edge   (std::string origin, std::string destination) const;
    T    edge_value (std::string origin, std::string destination) const;
    int  in_degree  (std::string node_name) const;
    int  out_degree (std::string node_name) const;
    int  degree     (std::string node_name) const;

    const ics::HashMap<std::string,LocalInfo>&                all_nodes () const;
    const ics::HashMap<ics::pair<std::string,std::string>,T>& all_edges () const;

    const ics::HashSet<std::string>& out_nodes(std::string node_name) const;
    const ics::HashSet<std::string>& in_nodes (std::string node_name) const;
    const ics::HashSet<ics::pair<std::string,std::string>>& out_edges (std::string node_name) const;
    const ics::HashSet<ics::pair<std::string,std::string>>& in_edges  (std::string node_name) const;

    //Operators
    HashGraph<T>& operator = (const HashGraph<T>& rhs);
    bool operator == (const HashGraph<T>& rhs) const;
    bool operator != (const HashGraph<T>& rhs) const;

  private:
    //All methods and operators relating to LocalInfo are defined below, followed by
    //  friend functions for insertion onto output streams of HashGrah and LocalInfo
    class LocalInfo {
      public:
        LocalInfo()                : from_graph(nullptr), out_nodes(hash_str), in_nodes(hash_str), out_edges(hash_pair_str), in_edges(hash_pair_str) {}
        LocalInfo(HashGraph<T>* g) : from_graph(g),       out_nodes(hash_str), in_nodes(hash_str), out_edges(hash_pair_str), in_edges(hash_pair_str) {}
        void connect(HashGraph<T>* g) {from_graph = g;}

        bool operator == (const LocalInfo& rhs) const {
          return this->in_nodes == rhs.in_nodes && this->out_nodes == rhs.out_nodes &&
                 this->in_edges == rhs.in_edges && this->out_edges == rhs.out_edges;
        }
        bool operator != (const LocalInfo& rhs) const {
          return !(*this == rhs);
        }

        //from_graph should point to the HashGraph LocalInfo is in, so LocalInfo
        //  methods (see <<)
        HashGraph<T>* from_graph;
        ics::HashSet<std::string>                        out_nodes;
        ics::HashSet<std::string>                        in_nodes;
        ics::HashSet<ics::pair<std::string,std::string>> out_edges;
        ics::HashSet<ics::pair<std::string,std::string>> in_edges;
   };//LocalInfo

    //These two friend << operators are easier to define here because of
    //  the nested LocalInfo in HashGraph, which both use; the nodes in HashGraph
    //  appear in alphabetical order; the four sets
    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const HashGraph<T2>& g) {
      if (g.empty()) {
        outs << "graph[]";
      }else{
        outs << "graph[\n";
        ics::HeapPriorityQueue<ics::pair<std::string,LocalInfo>> nodes(g.node_values.ibegin(),g.node_values.iend(), g.LocalInfo_gt);
        for (ics::pair<std::string,LocalInfo> x : nodes)
          outs << "  " << x << std::endl;
        outs << "]";
      }
      return outs;
    }

    friend std::ostream& operator << (std::ostream& outs, const LocalInfo& li) {
      outs << "LocalInfo[" << std::endl << "    out_nodes=" << li.out_nodes << std::endl;

      outs << "    out_edges=set[";
      if (li.out_edges.size() != 0) {
        ics::Iterator<ics::pair<std::string,std::string>>& i = li.out_edges.ibegin();
        outs << "->" << i->second << "(" << li.from_graph->edge_values[*i] << ")";
        ++i;
        for (/*See above*/; i != li.out_edges.iend(); ++i)
          outs << ",->" << i->second << "(" << li.from_graph->edge_values[*i] << ")";
        }
      outs << "]" << std::endl;

      outs << "    in_nodes =" << li.in_nodes << std::endl;

      outs << "    in_edges =set[";
      if (li.in_edges.size() != 0) {
        ics::Iterator<ics::pair<std::string,std::string>>& i = li.in_edges.ibegin();
        outs << i->first << "->(" << li.from_graph->edge_values[*i] << ")";
        ++i;
        for (/*See above*/; i != li.in_edges.iend(); ++i)
          outs << "," << i->first << "->(" << li.from_graph->edge_values[*i] << ")";
        }
      outs << "]" << std::endl << "  ]";
      return outs;
    }

    private:
      ics::HashMap<std::string,LocalInfo>                node_values;
      ics::HashMap<ics::pair<std::string,std::string>,T> edge_values;

      //Static methods for hashing (in the maps) and for printing in alphabetic
      //  order the nodes in a graph (see << for HashGraph<T>)
      static int hash_str(const std::string& s)
      {std::hash<std::string> str_hash; return str_hash(s);}

      static int hash_pair_str(const ics::pair<std::string,std::string>& s)
      {std::hash<std::string> str_hash; return str_hash(s.first)*str_hash(s.second);}

      static bool LocalInfo_gt(const ics::pair<std::string,LocalInfo>& a,
                              const ics::pair<std::string,LocalInfo>& b)
      {return a.first < b.first;}
};//HashGraph



//Default constructor
template<class T>
HashGraph<T>::HashGraph () : node_values(hash_str), edge_values(hash_pair_str)  {
	//std::cout << "..........Default Constructor*" << std::endl;


}


//Copy constructor
template<class T>
HashGraph<T>::HashGraph (HashGraph& g)  {//node_values(hash_str), edge_values(hash_pair_str) {
	//std::cout << "..........Copy Constructor*" << std::endl;
	node_values = ics::HashMap<std::string,LocalInfo> (g.node_values);
	edge_values = ics::HashMap<ics::pair<std::string,std::string>,T> (g.edge_values);

	for(auto kv : node_values)
	{
		kv.second.from_graph = this;
	}
}


//Add node_name to the graph if it is not already there.
//Ensure its from_graph refers to this graph.
template<class T>
void HashGraph<T>::add_node (std::string node_name) {
	//std::cout << "..........Add Node*" << std::endl;

	node_values.put(node_name, LocalInfo(this));

}


//Add an edge from origin to destination with value
//Add these node names and update edge_values and the LocalInfos of each node
template<class T>
void HashGraph<T>::add_edge (std::string origin, std::string destination, T value) {
	//std::cout << "..........Add Edge*" << std::endl;

	if( ! has_node(origin) ){add_node(origin);}
	if( ! has_node(destination) ){add_node(destination);}

	node_values[origin].out_nodes.insert(destination);
	node_values[destination].in_nodes.insert(origin);

	node_values[origin].out_edges.insert(ics::make_pair(origin, destination));
	node_values[destination].in_edges.insert(ics::make_pair(origin, destination));

	if( has_edge(origin, destination) )
	{
		return;
	}
	else
	{
		edge_values.put(ics::make_pair(origin, destination), value);
	}

}


//Remove all uses of node_name from the graph: update node_values, edge_values,
//  and all the LocalInfo in which it appears as an origin or destination node
//If the node_name is not in the graph, do nothing
//Hint: you cannot iterate over a set that you are changing, so you might have
// to copy a set and then iterate over it while removing values from the original set
template<class T>
void HashGraph<T>::remove_node (std::string node_name){
	//std::cout << "..........Remove Node*" << std::endl;

	if (has_node(node_name))
	{
		ics::HashSet<std::string> temp(hash_str);

		temp.insert(node_values[node_name].out_nodes.ibegin(), node_values[node_name].out_nodes.iend());
		temp.insert(node_values[node_name].in_nodes.ibegin(), node_values[node_name].in_nodes.iend());

		for( auto i : temp)
		{
			if(edge_values.has_key(ics::make_pair(i, node_name)))
			{
				remove_edge(i, node_name);
			}
			if(edge_values.has_key(ics::make_pair(node_name, i)))
			{
				remove_edge(node_name, i);
			}
			node_values[i].out_nodes.erase(node_name);
			node_values[i].in_nodes.erase(node_name);

		}
		node_values.erase(node_name);
	}
	else
	{
		return;
	}
}


//Remove all uses of this edge from the graph: update edge_values and all the
//  LocalInfo in which its origin and destination node appears
//If the edge is not in the graph, do nothing
//Hint: Simpler than remove_node: write and test this one first
template<class T>
void HashGraph<T>::remove_edge (std::string origin, std::string destination) {
	//std::cout << "..........Remove Edge*" << std::endl;

	if( edge_values.has_key(ics::make_pair(origin, destination)))
	{
		node_values[origin].out_edges.erase(ics::make_pair(origin, destination));
		node_values[destination].in_edges.erase(ics::make_pair(origin, destination));

		node_values[origin].out_nodes.erase(destination);
		node_values[destination].in_nodes.erase(origin);

		edge_values.erase(ics::make_pair(origin, destination));
	}
	else
	{
		return;
	}
}


//Clear the graph of all nodes and edges
template<class T>
void HashGraph<T>::clear() {
	//std::cout << "..........Clear*" << std::endl;
	node_values.clear();
	edge_values.clear();
}


//Load the nodes and edges for a graph from a text file whose form is
// (a) a node name (one per line, one any number of lines)
// (b) the word "NODESABOVEEDGESBELOW"
// (c) an origin node, destination node, and value (one triple per line,
//       with the values separated by separator, on any number of lines)
//Hint: use ics::split and istringstream (the extaction dual of ostreamstring)
template<class T>
void HashGraph<T>::load (std::ifstream& in_file, std::string separator) {
	//std::cout << "..........Load*" << std::endl;

	std::string line;
	while(getline(in_file, line))
	{
		if(line == "NODESABOVEEDGESBELOW"){break;}
		add_node(line);
	}
	while(getline(in_file, line))
	{
		std::vector<std::string> split_line = ics::split(line,separator);
		std::string origin = split_line[0];
		std::string dest   = split_line[1];
		std::string value = split_line[2];

		T int_val;

		std::istringstream(value) >> int_val;

		add_edge(origin, dest, int_val);
	}
	in_file.close();

}


//Store the nodes and edges in a graph into a text file whose form is specified
//  above for the load method; files written by store should be readable by load
//Hint: this is the easier of the two methods: write and test it first
template<class T>
void HashGraph<T>::store(std::ofstream& out_file, std::string separator) {
	//std::cout << "..........Store*" << std::endl;

	for( auto kv : node_values)
	{
		out_file << kv.first << std::endl;
	}

	out_file << "NODESABOVEEDGESBELOW" << std::endl;

	for (auto kv : edge_values)
	{
		out_file << kv.first.first << separator << kv.first.second << separator << kv.second << std::endl;
	}

	out_file.close();
}


//Returns whether a graph is empty
template<class T>
bool HashGraph<T>::empty() const {
	//std::cout << "..........Empty*" << std::endl;

	return node_values.empty();

}


//Returns the number of nodes in a graph
template<class T>
int HashGraph<T>::node_count() const {
	//std::cout << "..........Node Count*" << std::endl;
	return node_values.size();
}


//Returns the number of edges in a graph
template<class T>
int HashGraph<T>::edge_count() const {
	//std::cout << "..........Edge Count*" << std::endl;
	return edge_values.size();
}


//Returns whether or not node_name is in the graph
template<class T>
bool HashGraph<T>::has_node(std::string node_name) const {
	//std::cout << "..........Has Node*" << std::endl;
	return node_values.has_key(node_name);
}

//Returns whether or not the edge is in the graph
template<class T>
bool HashGraph<T>::has_edge(std::string origin, std::string destination) const {
	//std::cout << "..........Has Edge*" << std::endl;

	return edge_values.has_key(ics::make_pair(origin, destination));



}


//Returns the value of the edge in the graph; if the edge is not in the graph,
//  throw a GraphError exception with appropriate descriptive text
template<class T>
T HashGraph<T>::edge_value(std::string origin, std::string destination) const {
	//std::cout << "..........Edge Value*" << std::endl;

	if( edge_values.has_key(ics::make_pair(origin, destination)))
	{
		return edge_values[ics::make_pair(origin, destination)];
	}
	else
	{
		throw GraphError("EDGE VALUE ERROR: EDGE NOT IN GRAPH\n");
	}
}


//Returns the in-degree of node_name; if that node is not in the graph,
//  throw a GraphError exception with appropriate descriptive text
template<class T>
int HashGraph<T>::in_degree(std::string node_name) const {
	//std::cout << "..........In Degree*" << std::endl;

	if (has_node(node_name))
	{
		return node_values[node_name].in_edges.size();
	}
	else
	{
		throw GraphError("IN DEGREE: NODE NOT IN GRAPH\n");
	}
}


//Returns the out-degree of node_name; if that node is not in the graph,
//  throw a GraphError exception with appropriate descriptive text
template<class T>
int HashGraph<T>::out_degree(std::string node_name) const {
	//std::cout << "..........Out Degree*" << std::endl;

	if (has_node(node_name))
	{
		return node_values[node_name].out_edges.size();
	}
	else
	{
		throw GraphError("OUT DEGREE: NODE NOT IN GRAPH\n");
	}

}


//Returns the degree of node_name; if that node is not in the graph,
//  throw a GraphError exception with appropriate descriptive text.
template<class T>
int HashGraph<T>::degree(std::string node_name) const {
	//std::cout << "..........Degree*" << std::endl;

	if (has_node(node_name))
	{
		return node_values[node_name].out_edges.size() + node_values[node_name].in_edges.size();
	}
	else
	{
		throw GraphError("DEGREE: NODE NOT IN GRAPH\n");
	}
}


//Returns a reference to the all_nodes map;
//  the user should not mutate its data structure: call Graph commands instead
template<class T>
auto HashGraph<T>::all_nodes () const -> const ics::HashMap<std::string,LocalInfo>& {
	//std::cout << "..........All Nodes*" << std::endl;
	return node_values;
}


//Returns a reference to the all_edges map;
//  the user should not mutate its data structure: call Graph commands instead
template<class T>
const ics::HashMap<ics::pair<std::string,std::string>,T>& HashGraph<T>::all_edges () const {
	//std::cout << "..........All Edges*" << std::endl;
	return edge_values;
}

//Returns a reference to the out_nodes set in the LocalInfo for node_name;
//  the user should not mutate its data structure: call Graph commands instead; if
//  that node is not in the graph, throw a GraphError exception with appropriate
//  descriptive text
template<class T>
const ics::HashSet<std::string>& HashGraph<T>::out_nodes(std::string node_name) const {
	//std::cout << "..........Out Nodes*" << std::endl;

	if (has_node(node_name))
	{
		return node_values[node_name].out_nodes;
	}
	else
	{
		throw GraphError("OUT NODES: NODE NOT IN GRAPH\n");
	}

}


//Returns a reference to the in_nodes set in the LocalInfo for node_name;
//  the user should not mutate its data structure: call Graph commands instead; if
//  that node is not in the graph, throw a GraphError exception with appropriate
//  descriptive text
template<class T>
const ics::HashSet<std::string>& HashGraph<T>::in_nodes(std::string node_name) const{
	//std::cout << "..........In Nodes*" << std::endl;

	if (has_node(node_name))
	{
		return node_values[node_name].in_nodes;
	}
	else
	{
		throw GraphError("IN NODES: NODE NOT IN GRAPH\n");
	}
}


//Returns a reference to the out_edges set in the LocalInfo for node_name;
//  the user should not mutate its data structure: call Graph commands instead; if
//  that node is not in the graph, throw a GraphError exception with appropriate
//  descriptive text
template<class T>
const ics::HashSet<ics::pair<std::string,std::string>>& HashGraph<T>::out_edges (std::string node_name) const {
	//std::cout << "..........Out Edges*" << std::endl;
	if (has_node(node_name))
	{
		return node_values[node_name].out_edges;
	}
	else
	{
		throw GraphError("OUT EDGES: NODE NOT IN GRAPH\n");
	}
}


//Returns a reference to the in_edges set in the LocalInfo for node_name;
//  the user should not mutate its data structure: call Graph commands instead; if
//  that node is not in the graph, throw a GraphError exception with appropriate
//  descriptive text
template<class T>
const ics::HashSet<ics::pair<std::string,std::string>>& HashGraph<T>::in_edges (std::string node_name) const {
	//std::cout << "..........In Edges*" << std::endl;


	if (has_node(node_name))
	{
		return node_values[node_name].in_edges;
	}
	else
	{
		throw GraphError("IN EDGES: NODE NOT IN GRAPH\n");
	}
}


//Copy the specified graph into this and return the newly copied graph
//Hint: each copied LocalInfo object should reset from_graph to the this new graph
template<class T>
HashGraph<T>& HashGraph<T>::operator = (const HashGraph<T>& rhs){
	//std::cout << "..........= Operator" << std::endl;

	node_values = rhs.node_values;
	edge_values = rhs.edge_values;

	for(auto kv : node_values)
	{
		kv.second.from_graph = this;
	}

}


//Return whether two graphs are the same
template<class T>
bool HashGraph<T>::operator == (const HashGraph<T>& rhs) const{
	//std::cout << "..........== Operator*" << std::endl;

	return (node_values == rhs.node_values) && (edge_values == rhs.edge_values);
}


//Return whether two graphs are different
template<class T>
bool HashGraph<T>::operator != (const HashGraph<T>& rhs) const{
	//std::cout << "..........!= Operator*" << std::endl;
	return !(*this == rhs);
}

}

#endif /* HASH_GRAPH_HPP_ */
