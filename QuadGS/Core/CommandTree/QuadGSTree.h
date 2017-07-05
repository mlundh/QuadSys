/*
 * CommandBase.h
 *
 * Copyright (C) 2015 Martin Lundh
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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

/**
 * @class QuadGSTree
 * A container for values organized in a tree. The class represents one node.
 *
 * A node can be represented by a string:
 * nodeName<valueType>[value]
 *
 * It is possible to query the value or type of a node by accessing it
 * via only its name.
 */
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
    static const unsigned int mMaxNameLength;
    static const unsigned int mMaxDepth;
    static const unsigned int mMaxExtra;
    static const unsigned int mValueTypeLength;
    static const unsigned int mMaxDigitsInt32;
    static const unsigned int mMaxNodeLength;

    typedef std::shared_ptr<QuadGSTree> ptr;

    QuadGSTree(std::string name, QuadGSTreeValue::NodeType_t type);

    QuadGSTree(std::string path);

    virtual ~QuadGSTree();

    /**
     * Register new child node to current node. Will throw if there is a node
     * with the same name already.
     * @param path  A string containing the node information. Will use
     * the first node information if there is more than one present in the string.
     * @return  Name of the registered node.
     */
    std::string Register(const std::string path);

    /**
     * Get the name of this node.
     * @return  Name.
     */
    std::string GetName();

    /**
     * Set the parent of this node.
     * @param parent    Parent node.
     */
    void SetParent(QuadGSTree* parent);

    /**
     * Get the parent node. If there is no node, then GetParent will return null.
     * @return  Pointer to the parent.
     */
    QuadGSTree* GetParent();

    /**
     * Set value of current node. Extracts the value portion of the string and
     * tries to convert it to the value type of this node. Will throw if this
     * failed.
     * @param value Node string containing the value.
     * @return
     */
    virtual std::string SetValue(const std::string value);

    /**
     * Get the value of the node as a string.
     * @return  String containing the value.
     */
    virtual std::string GetValue();

    /**
     * Increase the value of the node by "value".
     * @param value Increase by this much.
     * @return
     */
    std::string ModifyValue(std::string value);

    /**
     * Get a shared pointer to this node.
     * @return  Pointer.
     */
    QuadGSTree::ptr GetSelf();

    /**
     * Find child node. Return pointer to child if found.
     * @param name      String describing node to be found.
     * @return          Pointer to child.
     */
    QuadGSTree::ptr Find(const std::string& name);

    /**
     * Checks if a node needs to be updated.
     * @param path  String describing this node, current node is evaluated against this string.
     * @return      True if this needs update.
     */
    bool NeedUpdate(const std::string& path);

    /**
     * Find all partial matches (children) to the string name, and return the matches
     * in the vector vec.
     * @param name  Partial name to match.
     * @param vec   return vector.
     */
    void FindPartial(const std::string& name, std::vector<std::string>& vec);

    /**
     * Get the number of child nodes this node has.
     * @return  Number of child nodes.
     */
    size_t getNrChildren();

    /**
     * List all children in a human readable format.
     * @return  String containing all children.
     */
    std::string ListChildren();

    /**
     * Get the node string of this node.
     * @return  Node string.
     */
    std::string GetNodeString();

    /**
     * Dump the tree to a string. The tree will be formatted according to the
     * spec. of a parameter QSF. The dump will end when the whole tree is dumped
     * or to length length.
     * @param tree              String containing the dumped tree.
     * @param startPosition     Array of starting values for dump at different depths.
     * @param depth             Current depth.
     * @param length            Max length of the string.
     * @return                  True if sucess, false otherwise.
     */
    bool DumpTree(std::string &tree, std::vector<size_t> &startPosition, unsigned int &depth, unsigned int length);

    /**
     * Print the tree to a string. The format is indented for each level and
     * should be used to display a human readable representation of this node
     * and all its child nodes, and so on.
     * @param depth Current depth.
     * @return      A formatted string.
     */
    std::string DumpTreeFormatted( size_t depth);


    /**
     * Remove the first module string from string s.
     * @param s String containing one or more modules. The first module
     * will get removed by this function.
     * @return  The removed module.
     */
    static std::string RemoveModuleString(std::string& s);

protected:

    std::string mName;
    QuadGSTree* mParent;
    std::vector< QuadGSTree::ptr > mChildren;
    QuadGSTreeValue mValue;
    Log mLogger;

    /**
     * Get the module name from a string.
     * @param s String containing the module.
     * @return  first name of the module in the string.
     */
    static std::string GetModuleName(const std::string& s);

    /**
     * Get the complete module string from a string. The incoming
     * string might contain many modules, this command copies the
     * first module string.
     * @param s     String containing one or more modules.
     * @return      Copy of s containing only the first module.
     */
    static std::string GetModuleString(const std::string& s);

    /**
     * Get the value type of string s.
     * @param s     A module string.
     * @return      The value type of module contained in s.
     */
    static std::string GetValueTypeString(const std::string& s);

    /***
     * Get the value string of module contained in s.
     * @param s     A module string.
     * @return      The value of module contained in s.
     */
    static std::string GetValueString(const std::string& s);

};


} /* namespace QuadGS */


#endif /* QUADGS_MODULES_CORE_COMMANDTREE_QUADGSTREE_H_ */
