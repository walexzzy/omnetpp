//=========================================================================
//  NODE.H - part of
//                          OMNeT++
//           Discrete System Simulation in C++
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2003 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef _NODE_H_
#define _NODE_H_

#include "util.h"


class Node;
class NodeType;
class Channel;
class DataflowManager;

/**
 * Represents a data element in an output vector file. Processing nodes
 * (Node) process elements of this type.
 *
 * @see Node, Channel, Port
 */
//FIXME rename to (t,val)
struct Datum
{
    double x;  //< usually t simulation time
    double y;  //< usually the value at t
};


/**
 * Connection point of channels in processing nodes.
 *
 * @see Datum, Node, Channel
 */
class Port
{
    private:
        Node *ownernode;
        Channel *chan;
    public:
        Port() {}
        Port(Node *ownerNode) {ownernode = ownerNode; chan = 0;}
        ~Port() {}
        void setNode(Node *ownerNode) {ownernode = ownerNode; chan = 0;}
        void setChannel(Channel *channel) {ASSERT(!chan); chan = channel;}
        Node *node() {return ownernode;}
        Channel *channel() const  {return chan;}
        Channel *operator()() const {ASSERT(chan); return chan;}
};


/**
 * Processing node. Processing nodes can be connected via ports and channels
 * to form a data flow network.
 *
 * @see DataflowManager, Port, Channel, Datum, NodeType
 */
class Node
{
        friend class DataflowManager;

    private:
        DataflowManager *mgr;
        NodeType *nodetype;

    protected:
        /**
         * Called when the node is added to the dataflow manager.
         */
        void setDataflowManager(DataflowManager *m) {mgr = m;}

    public:
        /**
         * Constructor
         */
        Node() {mgr=NULL; nodetype=NULL;}

        /**
         * Virtual destructor
         */
        virtual ~Node() {}

        DataflowManager *dataflowManager() const {return mgr;}

        void setNodeType(const NodeType *t)  {nodetype = const_cast<NodeType *>(t);}

        /**
         * Should be called right after construction by the corresponding
         * NodeType class.
         */
        NodeType *nodeType() const {return nodetype;}

        /** Execution and scheduling */
        //@{
        /**
         * Do some amount of work, then return
         */
        virtual void process() = 0;

        /**
         * Are more invocations if process() necessary()?
         */
        virtual bool finished() const = 0;

        /**
         * Provided it has not finished() yet, can process() be invoked
         * right now?
         */
        virtual bool isReady() const = 0;
        //@}
};

#endif


