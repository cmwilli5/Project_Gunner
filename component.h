#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "componentName.h"

class EntityManager;

class ComponentFactory;

namespace componentUtils {
    extern std::map<std::string, ComponentFactory*> factoryMap;
}

template <typename DataType>
DataType buildFromString(std::vector<std::string> str, std::vector<std::string>::size_type pos);

struct ComponentBase {
protected:
    ComponentBase();
    ~ComponentBase();

};

template <const std::string& cmpName, typename DataType>
struct Component;

class ComponentFactory {
public:

    virtual std::size_t build(EntityManager* manager, size_t idx, std::string instructions) = 0;
    virtual std::size_t build(EntityManager* manager, size_t idx, ComponentBase* cmp) = 0;
    virtual std::size_t build(EntityManager* manager, std::string instructions) = 0;
    virtual std::size_t build(EntityManager* manager, ComponentBase* cmp) = 0;

    virtual std::vector<std::string> tokenize(std::string instructions) = 0;

    virtual void deregisterManager(EntityManager*) = 0;

    virtual ~ComponentFactory() = 0;

protected:
    ComponentFactory();

};

template <const std::string& cmpName, typename DataType>
struct Component : public ComponentBase {

private:

    class ComponentFactoryInternal : public ComponentFactory {

    public:

        std::size_t build(EntityManager* manager, size_t idx, std::string instructions);

        std::size_t build(EntityManager* manager, size_t idx, ComponentBase* cmp);

        std::size_t build(EntityManager* manager, std::string instructions);

        std::size_t build(EntityManager* manager, ComponentBase* cmp);

        void deregisterManager(EntityManager*);

        std::vector<std::string> tokenize(std::string instructions);
    };

    typedef std::vector<Component<cmpName, DataType>> cmpPool;
public:
    static std::map<EntityManager*, cmpPool> componentPools;
    static const std::string getName();
    static ComponentFactoryInternal* factory;
    static void registerComponent();

    void build(std::vector<std::string> instructions);

    Component() : data{} {}
    Component(const Component& other) : data(other.data) {}
    Component(const DataType& newData) : data(newData) {}
    Component(Component&& other) : data(other.data) {}
    Component(DataType&& newData) : data(newData) {}
    Component(std::vector<std::string> instructions) : data{} {
        build(instructions);
    }

    Component& operator=(Component other) { data = other.data; return *this; }

    DataType data;

};

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, size_t idx, std::string instructions) {
    auto pool = Component<cmpName, DataType>::componentPools[manager];
    pool[idx].build(tokenize(instructions));
    return idx;

}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, size_t idx, ComponentBase* cmp) {
    auto pool = Component<cmpName, DataType>::componentPools[manager];
    pool[idx] = *static_cast<Component<cmpName, DataType>*>(cmp);
    return idx;
}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, std::string instructions) {
    auto pool = Component<cmpName, DataType>::componentPools[manager];
    pool.emplace_back(tokenize(instructions));
    return pool.size();
}

template <const std::string& cmpName, typename DataType>
std::size_t Component<cmpName, DataType>::ComponentFactoryInternal::build(EntityManager* manager, ComponentBase* cmp) {
    auto pool = Component<cmpName, DataType>::componentPools[manager];
    pool.emplace_back(*static_cast<Component<cmpName, DataType>*>(cmp));
    return pool.size();
}

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::ComponentFactoryInternal::deregisterManager(EntityManager* manager) {

    Component<cmpName, DataType>::componentPools.erase(manager);

}

template <const std::string& cmpName, typename DataType>
std::vector<std::string> Component<cmpName, DataType>::ComponentFactoryInternal::tokenize(std::string instructions) {
    auto beginCmp = instructions.find_first_of(" \n\f\r\t\v", instructions.find("component:" + cmpName));
    std::vector<std::string> tokenizedString;
    tokenizedString.reserve(5);
    int8_t i = 0;
    std::string::size_type tokenStart = std::string::npos;
    std::locale loc;
    if (beginCmp != std::string::npos) {
        tokenizedString.push_back("component:" + cmpName);
        for(auto start = beginCmp; start != std::string::npos &&
                start < instructions.size() && i < 4; ++start) {
            auto ch = instructions[start];
            if (!std::isspace(ch, loc) && ch != ',' && ch != '\n' && tokenStart == std::string::npos) {
                tokenStart = start;
            }
            if ( (std::isspace(ch, loc) || ch == ',' || ch== '\n') && tokenStart != std::string::npos) {
                tokenizedString.emplace_back(instructions.substr(tokenStart, start - tokenStart));
                ++i;
                tokenStart = std::string::npos;
            }

        }

        if (tokenStart != std::string::npos)
            tokenizedString.emplace_back(instructions.substr(tokenStart));

    }

    for( auto token : tokenizedString) {

    }

    return tokenizedString;

}

template <const std::string& cmpName, typename DataType>
typename Component<cmpName, DataType>::ComponentFactoryInternal* Component<cmpName, DataType>::factory{nullptr};

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::registerComponent() {
    if (!factory)
        factory = new ComponentFactoryInternal();
    componentUtils::factoryMap[cmpName] = factory;
}

template <const std::string& cmpName, typename DataType>
std::map<EntityManager*, std::vector<Component<cmpName, DataType>>> Component<cmpName, DataType>::componentPools{};

template <const std::string& cmpName, typename DataType>
const std::string Component<cmpName, DataType>::getName(){ return cmpName; };

template <const std::string& cmpName, typename DataType>
void Component<cmpName, DataType>::build(std::vector<std::string> instructions) {

    if (instructions.size() >= 2) {
            data = buildFromString<DataType>(instructions, 1);
    }

}

void deregisterAllComponents();

#endif // COMPONENT_H_INCLUDED
