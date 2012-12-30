// modified by Thatcher Ulrich for testing tu_gc::
//
// cl -Ox -DNDEBUG=1 -Zi -GX GCBench.cpp tu_gc_singlethreaded_marksweep.cpp -I.. -D_HAS_EXCEPTIONS=1 -DGC winmm.lib


// This is adapted from a benchmark written by John Ellis and Pete Kovac
// of Post Communications.
// It was modified by Hans Boehm of Silicon Graphics.
// Translated to C++ 30 May 1997 by William D Clinger of Northeastern Univ.
//
//      This is no substitute for real applications.  No actual application
//      is likely to behave in exactly this way.  However, this benchmark was
//      designed to be more representative of real applications than other
//      Java GC benchmarks of which we are aware.
//      It attempts to model those properties of allocation requests that
//      are important to current GC techniques.
//      It is designed to be used either to obtain a single overall performance
//      number, or to give a more detailed estimate of how collector
//      performance varies with object lifetimes.  It prints the time
//      required to allocate and collect balanced binary trees of various
//      sizes.  Smaller trees result in shorter object lifetimes.  Each cycle
//      allocates roughly the same amount of memory.
//      Two data structures are kept around during the entire process, so
//      that the measured performance is representative of applications
//      that maintain some live in-memory data.  One of these is a tree
//      containing many pointers.  The other is a large array containing
//      double precision floating point numbers.  Both should be of comparable
//      size.
//
//      The results are only really meaningful together with a specification
//      of how much memory was used.  It is possible to trade memory for
//      better time performance.  This benchmark should be run in a 32 MB
//      heap, though we don't currently know how to enforce that uniformly.
//
//      Unlike the original Ellis and Kovac benchmark, we do not attempt
//      measure pause times.  This facility should eventually be added back
//      in.  There are several reasons for omitting it for now.  The original
//      implementation depended on assumptions about the thread scheduler
//      that don't hold uniformly.  The results really measure both the
//      scheduler and GC.  Pause time measurements tend to not fit well with
//      current benchmark suites.  As far as we know, none of the current
//      commercial Java implementations seriously attempt to minimize GC pause
//      times.

#include <new.h>
#include <stdio.h>
//#include <sys/time.h>
#include <windows.h>

#ifdef GC
#  include "base/tu_gc_singlethreaded_marksweep.h"
#  include "base/tu_gc_singlethreaded_refcount.h"
#endif

//  These macros were a quick hack for the Macintosh.
//
//  #define currentTime() clock()
//  #define elapsedTime(x) ((1000*(x))/CLOCKS_PER_SEC)

#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

/* Get the current time in milliseconds */

// unsigned
// stats_rtclock( void )
// {
//   struct timeval t;
//   struct timezone tz;

//   if (gettimeofday( &t, &tz ) == -1)
//     return 0;
//   return (t.tv_sec * 1000 + t.tv_usec / 1000);
// }
unsigned stats_rtclock() {
	return timeGetTime();
}

static const int kStretchTreeDepth    = 18;      // about 16Mb
static const int kLongLivedTreeDepth  = 16;  // about 4Mb
static const int kArraySize  = 500000;  // about 4Mb
static const int kMinTreeDepth = 4;
static const int kMaxTreeDepth = 16;

#ifdef GC
DECLARE_GC_TYPES(tu_gc::singlethreaded_marksweep);
//DECLARE_GC_TYPES(tu_gc::singlethreaded_refcount);
#define GC_NEW(x)
struct Node0;
typedef gc_ptr<Node0> Node;
#else
typedef struct Node0 *Node;
#endif

struct Node0
#ifdef GC
	: public gc_object
#endif
{
        Node left;
        Node right;
        int i, j;
        Node0(const Node& l, const Node& r) { left = l; right = r; }
        Node0() { left = 0; right = 0; }
#       ifndef GC
          ~Node0() { if (left) delete left; if (right) delete right; }
#	endif
};

struct GCBench {

        // Nodes used by a tree of a given size
        static int TreeSize(int i) {
                return ((1 << (i + 1)) - 1);
        }

        // Number of iterations to use for a given tree depth
        static int NumIters(int i) {
                return 2 * TreeSize(kStretchTreeDepth) / TreeSize(i);
        }

        // Build tree top down, assigning to older objects.
        static void Populate(int iDepth, const Node& thisNode) {
                if (iDepth<=0) {
                        return;
                } else {
                        iDepth--;
#			ifndef GC
                          thisNode->left  = new Node0();
                          thisNode->right = new Node0();
#			else
                          thisNode->left  = new GC_NEW(Node0) Node0();
                          thisNode->right = new GC_NEW(Node0) Node0();
#			endif
                        Populate (iDepth, thisNode->left);
                        Populate (iDepth, thisNode->right);
                }
        }

        // Build tree bottom-up
        static Node MakeTree(int iDepth) {
                if (iDepth<=0) {
#		     ifndef GC
                        return new Node0();
#		     else
                        return new GC_NEW(Node0) Node0();
#		     endif
                } else {
#		     ifndef GC
                        return new Node0(MakeTree(iDepth-1),
                                         MakeTree(iDepth-1));
#		     else
                        return new GC_NEW(Node0) Node0(MakeTree(iDepth-1),
                                         		 MakeTree(iDepth-1));
#		     endif
                }
        }

        static void PrintDiagnostics() {
#if 0
                long lFreeMemory = Runtime.getRuntime().freeMemory();
                long lTotalMemory = Runtime.getRuntime().totalMemory();

                System.out.print(" Total memory available="
                                 + lTotalMemory + " bytes");
                System.out.println("  Free memory=" + lFreeMemory + " bytes");
#endif
        }

        static void TimeConstruction(int depth) {
                long    tStart, tFinish;
                int     iNumIters = NumIters(depth);
                Node    tempTree;

                printf("Creating %d trees of depth %d\n", iNumIters, depth);
                
                tStart = currentTime();
                for (int i = 0; i < iNumIters; ++i) {
#			ifndef GC
                          tempTree = new Node0();
#			else
                          tempTree = new GC_NEW(Node0) Node0();
#			endif
                        Populate(depth, tempTree);
#		        ifndef GC
                          delete tempTree;
#			endif
                        tempTree = 0;
                }
                tFinish = currentTime();
		printf("\tTop down construction took %d msec\n", elapsedTime(tFinish - tStart));
                     
                tStart = currentTime();
                for (int i = 0; i < iNumIters; ++i) {
                        tempTree = MakeTree(depth);
#			ifndef GC
                          delete tempTree;
#			endif
                        tempTree = 0;
                }
                tFinish = currentTime();
                printf("\tBottom up construction took %d msec\n", elapsedTime(tFinish - tStart));
        }

        void main() {
                Node    root;
                Node    longLivedTree;
                Node    tempTree;
                long    tStart, tFinish;
                long    tElapsed;

#ifdef GC
		// GC_full_freq = 30;
		//GC_enable_incremental();
#endif
		printf("Garbage Collector Test\n");
                printf(" Live storage will peak at %d bytes.\n\n",
		       2 * sizeof(Node0) * TreeSize(kLongLivedTreeDepth) +
		       sizeof(double) * kArraySize);
                printf(" Stretching memory with a binary tree of depth %d\n", kStretchTreeDepth);
                PrintDiagnostics();
               
                tStart = currentTime();
                
                // Stretch the memory space quickly
                tempTree = MakeTree(kStretchTreeDepth);
#		ifndef GC
                  delete tempTree;
#		endif
                tempTree = 0;

                // Create a long lived object
                printf(" Creating a long-lived binary tree of depth %d\n", kLongLivedTreeDepth);
#		ifndef GC
                  longLivedTree = new Node0();
#		else 
                  longLivedTree = new GC_NEW(Node0) Node0();
#		endif
                Populate(kLongLivedTreeDepth, longLivedTree);

                // Create long-lived array, filling half of it
                printf(" Creating a long-lived array of %d double\n", kArraySize);
#		ifndef GC
                  double *array = new double[kArraySize];
#		else
                  double *array = new double[kArraySize];
		  //                  double *array = (double *)
		  //				GC_MALLOC_ATOMIC(sizeof(double) * kArraySize);
#		endif
                for (int i = 0; i < kArraySize/2; ++i) {
                        array[i] = 1.0/i;
                }
                PrintDiagnostics();

                for (int d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2) {
                        TimeConstruction(d);
                }

                if (longLivedTree == 0 || array[1000] != 1.0/1000)
                        printf("Failed\n");
                                        // fake reference to LongLivedTree
                                        // and array
                                        // to keep them from being optimized away

                tFinish = currentTime();
                tElapsed = elapsedTime(tFinish-tStart);
                PrintDiagnostics();
                printf("Completed in %d msec\n", tElapsed);
#		ifdef GC
		//printf("Completed " << GC_gc_no << " collections" <<endl;
		//printf("Heap size is %d\n", GC_get_heap_size());
#		endif
        }
};

main () {
    GCBench x;
    x.main();
}
