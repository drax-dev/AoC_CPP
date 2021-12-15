#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <queue>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

std::string trim(std::string string_input)
{
	constexpr auto whitespaces = " \t\n\r\f\v";
	string_input.erase(string_input.find_last_not_of(whitespaces) + 1);
	string_input.erase(0,string_input.find_first_not_of(whitespaces));
	return string_input;
}

std::string read_file(const fs::path& path)
{
    std::ifstream f(path, std::ios::in | std::ios::binary);
    const auto sz = fs::file_size(path);
    std::string result(sz, '\0');
    f.read(result.data(), static_cast<long long>(sz));
    return result;
}

using graph = std::map<std::string, std::vector<std::string>>;

graph build_graph(const std::string& input_data)
{
	graph graph_instance;
    std::string first;
	std::string second;
    std::stringstream ss(input_data);
	while (std::getline(ss, first, '-')) 
    {
        std::getline(ss, second);
        second = trim(second);
		if(second != "start")
		{
			graph_instance[first].push_back(second);
		}
		if(first != "start" && first != "end")
		{
			graph_instance[second].push_back(first);
		}
		// std::cout << first << std::endl;
		// std::cout << second << std::endl;
    }
	graph_instance["end"].clear();

    return graph_instance;
}

void dfs(std::map<std::string, bool>& visited_nodes, graph& graph_instance, const std::string& node)
{
    if (!visited_nodes[node])
    {
    	std::cout << node << ", ";
		visited_nodes[node] = true;
		const auto node_neighbors = graph_instance[node];
		for (const auto& neighbor : node_neighbors)
		{
			dfs(visited_nodes, graph_instance, neighbor);
		}
	}
}

std::string traverse_path(std::map<std::string, bool>& visited_nodes, graph& graph_instance, const std::string& node, std::string path)
{
	path += node;
	if(node == "end")
	{
		return path;
	}

	const auto node_neighbors = graph_instance[node];
	for (const auto& neighbor : node_neighbors)
	{
		if (!visited_nodes[neighbor])
		{
			traverse_path(visited_nodes, graph_instance, neighbor, path);
			if (std::islower(neighbor[0]))
			{
				visited_nodes[neighbor] = true;
			}
		}
	}

	return path;
}

void traverse_graph(graph graph_instance)
{
	std::queue<std::string> node_queue;
	std::map<std::string, bool> visited_nodes;
	std::string path{};

	auto result = traverse_path(visited_nodes, graph_instance, "start", path);
	std::cout << result;
}


int main()
{
	const auto result = read_file("input_day12_small.txt");
    std::cout << result;
    auto graph_instance = build_graph(result);
	traverse_graph(graph_instance);
    std::cout<< "";
}