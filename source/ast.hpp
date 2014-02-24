/*-
 * Copyright (c) 2012, Achilleas Margaritis
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef AST_HPP
#define AST_HPP


#include <cassert>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include "parser.hpp"


namespace parserlib {


class ast_node;
template <class T, bool OPT> class ast_ptr;
template <class T> class ast_list;
template <class T> class ast;


/** type of AST node stack.
 */
typedef std::vector<ast_node *> ast_stack;


/** Base class for AST nodes.
 */
class ast_node {
public:
    ///constructor.
    ast_node() : parent_node(0) {}
    
    /** copy constructor.
        @param n source object.
     */
    ast_node(const ast_node &n) : parent_node(0) {}

    ///destructor.
    virtual ~ast_node() {}
    
    /** assignment operator.
        @param n source object.
        @return reference to this.
     */
    ast_node &operator = (const ast_node &n) { return *this; }
    
    /** get the parent node.
        @return the parent node, if there is one.
     */
    ast_node *parent() const { return parent_node; }
    
    /** interface for filling the contents of the node
        from a node stack.
        @param st stack.
     */
    virtual void construct(const input_range &r, ast_stack &st) {}
    
private:
    //parent
    ast_node *parent_node;    
    
    template <class T, bool OPT> friend class ast_ptr;
    template <class T> friend class ast_list;
    template <class T> friend class ast;
};


class ast_member;


/** type of ast member vector.
 */
typedef std::vector<ast_member *> ast_member_vector;


/** base class for AST nodes with children.
 */
class ast_container : public ast_node {
public:
    /** sets the container under construction to be this.
     */
    ast_container();

    /** sets the container under construction to be this.
        Members are not copied.
        @param src source object.
     */
    ast_container(const ast_container &src);

    /** the assignment operator.
        The members are not copied.
        @param src source object.
        @return reference to this.
     */
    ast_container &operator = (const ast_container &src) {
        return *this;
    }

    /** Asks all members to construct themselves from the stack.
        The members are asked to construct themselves in reverse order.
        from a node stack.
        @param st stack.
     */
    virtual void construct(const input_range &r, ast_stack &st);

private:
    ast_member_vector members;

    friend class ast_member;
};


/** Base class for children of ast_container.
 */
class ast_member {
public:
    /** automatically registers itself to the container under construction.
     */
    ast_member() { _init(); }

    /** automatically registers itself to the container under construction.
        @param src source object.
     */
    ast_member(const ast_member &src) { _init(); }

    /** the assignment operator.
        @param src source object.
        @return reference to this.
     */
    ast_member &operator = (const ast_member &src) {
        return *this;
    }
    
    /** returns the container this belongs to.
        @return the container this belongs to.
     */
    ast_container *container() const { return container_node; }

    /** interface for filling the the member from a node stack.
        @param st stack.
     */
    virtual void construct(const input_range &r, ast_stack &st) = 0;

private:
    //the container this belongs to.
    ast_container *container_node;

    //register the AST member to the current container.
    void _init();
};


/** pointer to an AST object.
    It assumes ownership of the object.
    It pops an object of the given type from the stack.
    @param T type of object to control.
    @param OPT if true, the object becomes optional.
 */
template <class T, bool OPT = false> class ast_ptr : public ast_member {
public:
    /** the default constructor.
        @param obj object.
     */
    ast_ptr(T *obj = 0) : ptr(obj) {
        _set_parent();
    }

    /** the copy constructor.
        It duplicates the underlying object.
        @param src source object.
     */
    ast_ptr(const ast_ptr<T, OPT> &src) :
        ptr(src.ptr ? new T(*src.ptr) : 0)
    {
        _set_parent();
    }

    /** deletes the underlying object.
     */
    ~ast_ptr() {
        delete ptr;
    }

    /** copies the given object.
        The old object is deleted.
        @param obj new object.
        @return reference to this.
     */
    ast_ptr<T, OPT> &operator = (const T *obj) {
        delete ptr;
        ptr = obj ? new T(*obj) : 0;
        _set_parent();
        return *this;
    }

    /** copies the underlying object.
        The old object is deleted.
        @param src source object.
        @return reference to this.
     */
    ast_ptr<T, OPT> &operator = (const ast_ptr<T, OPT> &src) {
        delete ptr;
        ptr = src.ptr ? new T(*src.ptr) : 0;
        _set_parent();
        return *this;
    }

    /** gets the underlying ptr value.
        @return the underlying ptr value.
     */
    T *get() const {
        return ptr;
    }

    /** auto conversion to the underlying object ptr.
        @return the underlying ptr value.
     */
    operator T *() const {
        return ptr;
    }

    /** member access.
        @return the underlying ptr value.
     */
    T *operator ->() const {
        assert(ptr);
        return ptr;
    }

    /** Pops a node from the stack.
        @param st stack.
        @exception std::logic_error thrown if the node is not of the appropriate type;
            thrown only if OPT == false or if the stack is empty.
     */
    virtual void construct(const input_range &r, ast_stack &st) {
        //check the stack node
        //if (st.empty()) throw std::logic_error("empty AST stack");
    
        //get the node
        ast_node *node = st.back();
        
        //get the object
#if defined(__GXX_RTTI)
        T *obj = dynamic_cast<T *>(node);
#else
        T *obj = static_cast<T *>(node);
#endif
        
        //if the object is optional, simply return
        if (OPT) {
            if (!obj) return;
        }
        
        //else if the object is mandatory, throw an exception
        else {
            //if (!obj) throw std::logic_error("invalid AST node");
        }
        
        //pop the node from the stack
        st.pop_back();
        
        //set the new object
        delete ptr;
        ptr = obj;
        _set_parent();
    }

private:
    //ptr
    T *ptr;
    
    //set parent of object
    void _set_parent() {
        if (ptr) ptr->parent_node = container();
    }
};


/** A list of objects.
    It pops objects of the given type from the ast stack, until no more objects can be popped.
    It assumes ownership of objects.
    @param T type of object to control.
 */
template <class T> class ast_list : public ast_member {
public:
    ///list type.
    typedef std::list<T *> container;

    ///the default constructor.
    ast_list() {}

    /** duplicates the objects of the given list.
        @param src source object.
     */
    ast_list(const ast_list<T> &src) {
        _dup(src);
    }

    /** deletes the objects.
     */
    ~ast_list() {
        _clear();
    }

    /** deletes the objects of this list and duplicates the given one.
        @param src source object.
        @return reference to this.
     */
    ast_list<T> &operator = (const ast_list<T> &src) {
        if (&src != this) {
            _clear();
            _dup(src);
        }
        return *this;
    }

    /** returns the container of objects.
        @return the container of objects.
     */
    const container &objects() const {
        return child_objects;
    }

    /** Pops objects of type T from the stack until no more objects can be popped.
        @param st stack.
     */
    virtual void construct(const input_range &r, ast_stack &st) {
        for(;;) {
            //if the stack is empty
            if (st.empty()) break;
            
            //get the node
            ast_node *node = st.back();
            
            //get the object
#if defined(__GXX_RTTI)
            T *obj = dynamic_cast<T *>(node);
#else
            T *obj = static_cast<T *>(node);
#endif
            
            //if the object was not not of the appropriate type,
            //end the list parsing
            if (!obj) return;
            
            //remove the node from the stack
            st.pop_back();
            
            //insert the object in the list, in reverse order
            child_objects.push_front(obj);
            
            //set the object's parent
            obj->parent_node = ast_member::container();
        }
    }

private:
    //objects
    container child_objects;

    //deletes the objects of this list.
    void _clear() {
        while (!child_objects.empty()) {
            delete child_objects.back();
            child_objects.pop_back();
        }
    }

    //duplicate the given list.
    void _dup(const ast_list<T> &src) {
        for(typename container::const_iterator it = src.child_objects.begin();
            it != src.child_objects.end();
            ++it)
        {
            T *obj = new T(*it);
            child_objects.push_back(obj);
            obj->parent_node = ast_member::container();
        }
    }
};

/** parses the given input.
    @param i input.
    @param g root rule of grammar.
    @param ws whitespace rule.
    @param el list of errors.
    @param d user data, passed to the parse procedures.
    @return pointer to ast node created, or null if there was an error.
        The return object must be deleted by the caller.
 */
ast_node *parse(Input &i, rule &g, rule &ws, error_list &el, const ParserDelegate &d);


class ASTParserDelegate : ParserDelegate
{
	std::unordered_map<rule*, parse_proc> handlers;
	void set_parse_proc(rule &r, parse_proc p);
	public:
	ASTParserDelegate();
	virtual parse_proc get_parse_proc(rule &) const;
	static void bind_parse_proc(rule &r, parse_proc p);
	template <class T> bool parse(Input &i, rule &g, rule &ws, error_list &el, T *&ast) const
	{
		ast_node *node = parserlib::parse(i, g, ws, el, *this);
#if defined(__GXX_RTTI)
		ast = dynamic_cast<T *>(node);
#else
		ast = static_cast<T *>(node);
#endif
		if (ast) return true;
		delete node;
		return false;
	}
};

/** AST function which creates an object of type T
    and pushes it to the node stack.
 */
template <class T> class ast {
public:
	/** constructor.
		@param r rule to attach the AST function to.
	 */
	ast(rule &r)
	{
		ASTParserDelegate::bind_parse_proc(r, [](const pos &b, const pos &e, void *d)
			{
				ast_stack *st = reinterpret_cast<ast_stack *>(d);
				T *obj = new T();
				obj->construct(input_range(b, e), *st);
				st->push_back(obj);
			});
	}
};


} //namespace parserlib


#endif //AST_HPP
