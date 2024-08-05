#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <algorithm>

using namespace std;

class Node
{
public:
    string name;
    vector<pair<Node*, string>> subscriptions;
    map<string, pair<int, int>> event_data;

    Node(const string& name) : name(name) {}

    void subscribe(Node* sender, const string& type)
    {
        subscriptions.push_back({ sender, type });
    }

    void unsubscribe(Node* sender)
    {
        subscriptions.erase(remove_if(subscriptions.begin(), subscriptions.end(),
            [sender](const pair<Node*, string>& sub)
            {
                return sub.first == sender;
            }),
            subscriptions.end());
    }

    void create_event(int value) 
    {
        for (const auto& sub : subscriptions)
        {
            sub.first->process_event(name, value, sub.second);
        }
    }

    void process_event(const string& sender, int value, const string& type)
    {
        if (type == "sum")
        {
            event_data[sender].first += value;
            cout << sender << " -> " << name << ": S = " << event_data[sender].first << endl;
        }
        else if (type == "count") {
            event_data[sender].second++;
            cout << sender << " -> " << name << ": N = " << event_data[sender].second << endl;
        }
    }
};

class Network 
{
public:
    vector<Node*> nodes;

    void update(double create_event_prob, double subscribe_prob, double unsubscribe_prob, double create_node_prob) 
    {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<double> dist(0.0, 1);

        for (auto it = nodes.begin(); it != nodes.end();)
        {
            Node* node = *it;
            double rand_val = dist(gen);

            if (rand_val < create_event_prob)
            {
                int value = uniform_int_distribution<int>(1, 100)(gen);
                node->create_event(value);
            }
            else if (rand_val < create_event_prob + subscribe_prob) 
            {
                if (!nodes.empty()) {
                    Node* sender = nodes[uniform_int_distribution<int>(0, nodes.size() - 1)(gen)];
                    if (sender != node)
                    {
                        string type = (dist(gen) < 0.5) ? "sum" : "count";
                        node->subscribe(sender, type);
                    }
                }
            }
            else if (rand_val < create_event_prob + subscribe_prob + unsubscribe_prob) 
            {
                if (!node->subscriptions.empty())
                {
                    auto& sub = node->subscriptions[uniform_int_distribution<int>(0, node->subscriptions.size() - 1)(gen)];
                    node->unsubscribe(sub.first);
                }
            }
            else if (rand_val < create_event_prob + subscribe_prob + unsubscribe_prob + create_node_prob)
            {
                string new_name = "Node " + to_string(nodes.size() + 1);
                Node* new_node = new Node(new_name);
                nodes.push_back(new_node);
                node->subscribe(new_node, (dist(gen) < 0.5) ? "sum" : "count");
            }

            if (node->subscriptions.empty())
            {
                // Unsubscribe other nodes from this node's events
                for (auto& other_node : nodes)
                {
                    if (other_node != node)
                    {
                        other_node->unsubscribe(node);
                    }
                }

                delete node;
                it = nodes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};

int main() {
    Network network;

    // Create nodes
    for (int i = 1; i <= 5; i++)
    {
        string name = "Node " + to_string(i);
        network.nodes.push_back(new Node(name));
    }

    // Simulate updates
    for (int i = 0; i < 10; i++)
    {
        cout << "Update " << i + 1 << ":\n";
        network.update(0.7, 0.7, 0.1, 0.1);
        cout << endl;
    }

    // Clean up nodes
    for (auto it = network.nodes.begin(); it != network.nodes.end(); )
    {
        Node* node = *it;

        for (auto& other_node : network.nodes)
        {
            if (other_node != node)
            {
                other_node->unsubscribe(node);
            }
        }

        delete node;
        it = network.nodes.erase(it);
    }

    return 0;
}
