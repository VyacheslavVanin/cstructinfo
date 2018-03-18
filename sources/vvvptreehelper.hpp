#ifndef VVVPTREEHELPER_HPP
#define VVVPTREEHELPER_HPP
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

template <typename DATA_TYPE, typename PATH_TYPE>
void ptree_add_value(boost::property_tree::ptree& tree, PATH_TYPE&& path,
                     DATA_TYPE&& data)
{
    tree.put(std::forward<PATH_TYPE>(path), std::forward<DATA_TYPE>(data));
}

template <typename NAME_TYPE, typename NODE>
void ptree_add_subnode(boost::property_tree::ptree& tree, NAME_TYPE&& name,
                       NODE&& subnode)
{
    tree.push_back(std::make_pair(std::forward<NAME_TYPE>(name),
                                  std::forward<NODE>(subnode)));
}

template <typename DATA>
void ptree_array_add_values(boost::property_tree::ptree& arr, DATA&& data)
{
    boost::property_tree::ptree arr_elem1;
    arr_elem1.put("", std::forward<DATA>(data));
    arr.push_back(std::make_pair("", std::move(arr_elem1)));
}

template <typename DATA, typename... ARGS>
void ptree_array_add_values(boost::property_tree::ptree& arr, DATA&& data,
                            ARGS&&... args)
{
    boost::property_tree::ptree arr_elem1;
    arr_elem1.put("", std::forward<DATA>(data));
    arr.push_back(std::make_pair("", std::move(arr_elem1)));
    ptree_array_add_values(arr, std::forward<ARGS>(args)...);
}

template <typename NODE>
void ptree_array_add_node(boost::property_tree::ptree& arr, NODE&& node)
{
    arr.push_back(std::make_pair("", std::forward<NODE>(node)));
}

#endif
