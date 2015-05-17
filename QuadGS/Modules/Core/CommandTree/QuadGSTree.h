/*
 * CommandBase.h
 *
 *  Created on: Apr 26, 2015
 *      Author: martin
 */

#ifndef QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREE_H_
#define QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREE_H_


#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "QuadGSTreeValue.h"

namespace QuadGS {

class QuadGSTree
{
public:

    typedef std::shared_ptr<QuadGSTree> ptr;

    QuadGSTree(std::string name, QuadGSTreeValue::NodeType_t type = QuadGSTreeValue::NodeType_t::NoType);

    virtual ~QuadGSTree();

    void Register(std::string name);

    std::string GetName();

    void SetParent(QuadGSTree* parent);

    QuadGSTree* GetParent();

    virtual std::string SetValue(std::string path, std::string value);

    virtual std::string GetValue(std::string path);

    int Find(std::string name);

    void FindPartial(std::string& name, std::string& parent_name, std::vector<std::string>& vec);

    size_t getNrChildren();

    std::string DumpTree(std::string path, size_t depth);


protected:

    std::string mName;
    QuadGSTree* mParent;
    const std::string mDelimiter;
    const std::string mValueDelimiterFront;
    const std::string mValueDelimiterBack;
    std::vector< QuadGSTree::ptr > mChildren;
    QuadGSTreeValue mValue;

    std::string GetModuleString(std::string& s);
    std::string GetValueString(std::string& s);
    bool FollowPath(const std::string& s);
};


} /* namespace QuadGS */


#endif /* QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREE_H_ */
