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
#include "Log.h"

#include "QuadGSTreeValue.h"

namespace QuadGS {

class QuadGSTree : public std::enable_shared_from_this<QuadGSTree>
{
public:

  static const std::string mBranchDelimiter;
  static const std::string mParentDelimiter;
  static const std::string mSelfDelimiter;
  static const std::string mValueTypeDelimiterFront;
  static const std::string mValueTypeDelimiterBack;
  static const std::string mValueDelimiterFront;
  static const std::string mValueDelimiterBack;

    typedef std::shared_ptr<QuadGSTree> ptr;

    QuadGSTree(std::string name, QuadGSTreeValue::NodeType_t type);

    QuadGSTree(std::string path);

    virtual ~QuadGSTree();

    std::string Register(const std::string path);

    std::string GetName();

    void SetParent(QuadGSTree* parent);

    QuadGSTree* GetParent();

    virtual std::string SetValue(const std::string value);

    virtual std::string GetValue();
    
    QuadGSTree::ptr GetSelf();

    QuadGSTree::ptr Find(const std::string& name, bool findFull = true);

    bool NeedUpdate(const std::string& path);

    void FindPartial(const std::string& name, std::vector<std::string>& vec);

    size_t getNrChildren();
    
    std::string ListChildren();

    std::string GetNodeString();

    std::string DumpTree();

    std::string DumpTreeFormatted( size_t depth);


    static std::string RemoveModuleString(std::string& s);

protected:

    std::string mName;
    QuadGSTree* mParent;
    std::vector< QuadGSTree::ptr > mChildren;
    QuadGSTreeValue mValue;
    Log mLogger;
    static std::string GetModuleName(const std::string& s);
    static std::string GetModuleString(const std::string& s);
    static std::string GetValueTypeString(const std::string& s);
    static std::string GetValueString(const std::string& s);

};


} /* namespace QuadGS */


#endif /* QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREE_H_ */
