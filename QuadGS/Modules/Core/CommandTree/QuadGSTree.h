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

class QuadGSTree : public std::enable_shared_from_this<QuadGSTree>
{
public:

  static const std::string mBranchDelimiter;
  static const std::string mParentDelimiter;
  static const std::string mValueTypeDelimiterFront;
  static const std::string mValueTypeDelimiterBack;
  static const std::string mValueDelimiterFront;
  static const std::string mValueDelimiterBack;

    typedef std::shared_ptr<QuadGSTree> ptr;

    QuadGSTree(std::string name, QuadGSTreeValue::NodeType_t type = QuadGSTreeValue::NodeType_t::NoType);

    virtual ~QuadGSTree();

    std::string Register(std::string name);

    std::string GetName();

    void SetParent(QuadGSTree* parent);

    QuadGSTree* GetParent();

    virtual std::string SetValue(std::string value);

    virtual std::string GetValue();
    
    QuadGSTree::ptr GetSelf();

    QuadGSTree::ptr Find(std::string& name, bool findFull = true);

    void FindPartial(std::string& name, std::vector<std::string>& vec);

    size_t getNrChildren();
    
    std::string ListChildren();

    std::string DumpTree( size_t depth);


protected:

    std::string mName;
    QuadGSTree* mParent;
    std::vector< QuadGSTree::ptr > mChildren;
    QuadGSTreeValue mValue;

    std::string GetModuleString(std::string& s);
    std::string GetValueTypeString(std::string& s);
    std::string GetValueString(std::string& s);
};


} /* namespace QuadGS */


#endif /* QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREE_H_ */
