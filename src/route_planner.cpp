#include "route_planner.h"
#include <algorithm>

RoutePlanner::RoutePlanner(RouteModel &model, float start_x, float start_y, float end_x, float end_y): m_Model(model)
                                                                            , start_node(nullptr), end_node(nullptr) {
    // Convert inputs to percentage:
    start_x *= 0.01;
    start_y *= 0.01;
    end_x *= 0.01;
    end_y *= 0.01;

    start_node = &m_Model.FindClosestNode(start_x, start_y);
    end_node = &m_Model.FindClosestNode(end_x, end_y);


}

float RoutePlanner::CalculateHValue(RouteModel::Node const *node) {
    return end_node->distance(*node);
}

void RoutePlanner::AddNeighbors(RouteModel::Node *current_node) {
    
    if(!current_node)
        return;

    current_node->FindNeighbors();

    for(RouteModel::Node* neighbor : current_node->neighbors){
        neighbor->parent = current_node;
        neighbor->h_value = CalculateHValue(neighbor);
        neighbor->g_value = current_node->g_value + current_node->distance(*neighbor);
        neighbor->visited = true;
        open_list.push_back(neighbor);
    }
}

RouteModel::Node *RoutePlanner::NextNode() {

    RouteModel::Node* current = nullptr;

    if(!open_list.empty()) {

        std::sort(open_list.begin(), open_list.end(), [](auto const* node1, auto const* node2) {
            return node1->h_value + node1->g_value > node2->h_value + node2->g_value;
        });

        current = open_list.back();
        open_list.pop_back();
    }

    return current;
}

std::vector<RouteModel::Node> RoutePlanner::ConstructFinalPath(RouteModel::Node *current_node) {

    if(!current_node)
        return {};

    // Create path_found vector
    distance = 0.0f;
    std::vector<RouteModel::Node> path_found;

    while (current_node && current_node != start_node)
    {
        distance += current_node->distance(*(current_node->parent));
        path_found.push_back(RouteModel::Node(*current_node));  // Default copy constructor
        current_node = current_node->parent;
    }

    path_found.push_back(RouteModel::Node(*start_node));  // Default copy constructor
    
    std::reverse(path_found.begin(), path_found.end());

    distance *= m_Model.MetricScale(); // Multiply the distance by the scale of the map to get meters.
    return path_found;

}

void RoutePlanner::AStarSearch() {
    RouteModel::Node *current_node = nullptr;

    current_node = start_node;
    current_node->visited = true;

    while (current_node != end_node)
    {
       AddNeighbors(current_node);
       current_node = NextNode();
    }
    
    m_Model.path = ConstructFinalPath(end_node);
}