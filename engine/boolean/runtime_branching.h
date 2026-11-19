/*
 * boolean/runtime_branching.h
 *
 */

#pragma once

namespace GC
{

class RuntimeBranching
{
    bool tainted;

public:
	RuntimeBranching() : tainted(true)
	{
	}

    void untaint()
    {
#ifdef DEBUG_YAO
        cout << "untaint from " << tainted << endl;
#endif
        bool was_tainted = tainted;
        tainted = false;
        if (was_tainted)
            throw NeedsCleaning();
        else
            taint();
    }
    void taint()
    {
        tainted = true;
    }

    bool is_tainted()
    {
        return tainted;
    }
};

} /* namespace GC */
