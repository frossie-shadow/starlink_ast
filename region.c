/*
*class++
*  Name:
*     Region

*  Purpose:
*     Represents a region within a coordinate system.

*  Constructor Function:
c     astRegion
f     AST_REGION

*  Description:
*     This class provides the basic facilities for describing a region within 
*     a specified coordinate system. However, the Region class does not
*     have a constructor function of its own, as it is simply a container 
*     class for a family of specialised Regions which implement Regions
*     with particular shapes.
*
*     All sub-classes of Region require a Frame to be supplied when the Region
*     is created. This Frame describes the coordinate system in which the
*     Region is defined, and is referred to as the "encapsulated Frame" below. 
*     Constructors will also typically required one or more positions to be 
*     supplied which define the location and extent of the region. These 
*     positions must be supplied within the encapsulated Frame.
*
*     The Region class inherits from the Frame class, and so a Region can be 
*     supplied where-ever a Frame is expected. In these cases, supplying a 
*     Region is equivalent to supplying a reference to its encapsulated Frame. 
*     Thus all the methods of the Frame class can be used on the Region class. 
*     For instance, the 
c     astFormat function
f     AST_FORMAT routine
*     may be used on a Region to format an axis value. 
*
*     In addition, since Frame inherits from Mapping, a Region is also a sort 
*     of Mapping. Transforming positions by supplying a Region to one of the
c     astTran<X> functions
f     AST_TRAN<X> routines
*     is the way to determine if a given position is inside or outside the 
*     Region. When used as a Mapping, most classes of Frame are equivalent to 
*     a UnitMap. However, the Region class modifies this behaviour so that a 
*     Region acts like a UnitMap only for input positions which are within the 
*     area represented by the Region. Input positions which are outside the 
*     area produce bad output values (i.e. the output values are equal to 
*     AST__BAD). This behaviour is the same for both the forward and the 
*     inverse transformation. In this sense the "inverse transformation"
*     is not a true inverse of the forward transformation, since applying
*     the forward transformation to a point outside the Region, and then
*     applying the inverse transformation results, in a set of AST__BAD axis
*     values rather than the original axis values.
*
*     If the coordinate system represented by the Region is changed (by
*     changing the values of one or more of the attribute which the Region
*     inherits from its encapsulated Frame), the area represented by 
*     the Region is mapped into the new coordinate system. For instance, let's 
*     say a Circle (a subclass of Region) is created, a SkyFrame being
*     supplied to the constructor so that the Circle describes a circular 
*     area on the sky in FK4 equatorial coordinates. Since Region inherits 
*     from Frame, the Circle will have a System attribute and this attribute 
*     will be set to "FK4". If the System attribute of the Region is then 
*     changed from FK4 to FK5, the circular area represented by the Region 
*     will automatically be mapped from the FK4 system into the FK5 system.
*     In general, changing the coordinate system in this way may result in the 
*     region changing shape - for instance, a circle may change into an 
*     ellipse if the transformation from the old to the new coordinate system 
*     is linear but with different scales on each axis. Thus the specific 
*     class of a Region cannot be used as a guarantee of the shape in any
*     particular coordinate system. If the 
c     astSimplify function
f     AST_SIMPLIFY routine
*     is used on a Region, it will endeavour to return a new Region of 
*     a sub-class which accurately describes the shape in the current
*     coordinate system of the Region (but this may not always be possible).
*
*     It is possible to negate an existing Region so that it represents all 
*     areas of the encapsulated Frame except for the area specified when 
*     the Region was created.

*  Inheritance:
*     The Region class inherits from the Frame class.

*  Attributes:
*     In addition to those attributes common to all Frames, every
*     Region also has the following attributes:
*
*     - Negated: Has the original region been negated?
*     - Closed: Should the boundary be considered to be inside the region?
*     - MeshSize: Number of points used to create a mesh covering the Region
*     - FillFactor: Fraction of the Region which is of interest
*     - Bounded: Is the Region bounded?
*
*     Every Region also inherits any further attributes that belong
*     to the encapsulated Frame, regardless of that Frame's class. (For
*     example, the Equinox attribute, defined by the SkyFrame class, is
*     inherited by any Region which represents a SkyFrame.) 

*  Functions:
c     In addition to those functions applicable to all Frames, the
c     following functions may also be applied to all Regions:
f     In addition to those routines applicable to all Frames, the
f     following routines may also be applied to all Regions:
*
c     - astGetRegionFrame: Get a copy of the Frame represent by a Region
f     - AST_GETREGIONFRAME: Get a copy of the Frame represent by a Region
c     - astMapRegion: Transform a Region into a new coordinate system.
f     - AST_MAPREGION: Transform a Region into a new coordinate system.
c     - astNegate: Toggle the value of the Negated attribute
f     - AST_NEGATE: Toggle the value of the Negated attribute
c     - astOverlap: Determines the nature of the overlap between two Regions
f     - AST_OVERLAP: Determines the nature of the overlap between two Regions
c     - astMask<X>: Mask a region of a data grid
f     - AST_MASK<X>: Mask a region of a data grid
c     - astSetUnc: Associate a new uncertainty with a Region
f     - AST_SETUNC: Associate a new uncertainty with a Region

*  Copyright:
*     <COPYRIGHT_STATEMENT>

*  Authors:
*     DSB: David S. Berry (STARLINK)

*  History:
*     3-DEC-2003 (DSB):
*        Original version.
*class--

*  Implementation Notes:
*     - Internal and external bounding boxes need to be maintained: These can
*     speed up the evaluation of whether a point is inside or outside a region.
*     - All sub-classes must over-ride the following abstract methods declared 
*     in this class: astRegBaseBox, astRegBaseMesh, astRegPins, astRegCentre. 
*     They must also extend the astTransform method. In addition they should 
*     usually extend astSimplify.

*/

/* Module Macros. */
/* ============== */
/* Set the name of the class we are implementing. This indicates to
   the header files that define class interfaces that they should make
   "protected" symbols available. */
#define astCLASS Region

/* Macros which return the maximum and minimum of two values. */
#define MAX(aa,bb) ((aa)>(bb)?(aa):(bb))
#define MIN(aa,bb) ((aa)<(bb)?(aa):(bb))

/* Macro to check for equality of floating point values. We cannot
   compare bad values directory because of the danger of floating point
   exceptions, so bad values are dealt with explicitly. */
#define EQUAL(aa,bb) (((aa)==AST__BAD)?(((bb)==AST__BAD)?1:0):(((bb)==AST__BAD)?0:(fabs((aa)-(bb))<=1.0E5*MAX((fabs(aa)+fabs(bb))*DBL_EPSILON,DBL_MIN))))

/*
*  Name:
*     MAKE_CLEAR

*  Purpose:
*     Define a function to clear an attribute value for a Region.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_CLEAR(attribute)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static void Clear<Attribute>( AstFrame *this )
*
*     that clears the value of a specified attribute for the encapsulated
*     FrameSet within a Region (this). This function is intended to over-ride 
*     the astClear<Attribute> method inherited from the Frame class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*/

/* Define the macro. */
#define MAKE_CLEAR(attribute) \
static void Clear##attribute( AstFrame *this_frame ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to the Region structure */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Obtain a pointer to the encapsulated FrameSet and invoke its \
   astClear method. The protected astClear##attribute method is not used \
   because we want the current Frame of the FrameSet tp be re-mapped if \
   necessary. */ \
   astClear( this->frameset, #attribute ); \
}

/*
*  Name:
*     MAKE_CLEAR_AXIS

*  Purpose:
*     Define a function to clear an attribute value for a Region axis.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_CLEAR_AXIS(attribute)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static void Clear<Attribute>( AstFrame *this, int axis )
*
*     that clears the value of a specified attribute for an axis of
*     the encapsulated FrameSet within a Region (this). This function is
*     intended to over-ride the astClear<Attribute> method inherited
*     from the Frame class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*/

/* Define the macro. */
#define MAKE_CLEAR_AXIS(attribute) \
static void Clear##attribute( AstFrame *this_frame, int axis ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to the Region structure */ \
   char buf[100];              /* Buffer for attribute name */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Validate the axis index supplied. */ \
   (void) astValidateAxis( this, axis, "astClear" #attribute ); \
\
/* We use the public astSetx method rather than the protected \
   astSet#attribute method so that the current Frame in the encapsulated \
   FrameSet will be re-mapped if necessary. Construct the attribute name. */ \
   sprintf( buf, "%s(%d)", #attribute, axis + 1 ); \
\
/* Obtain a pointer to the Region's encapsulated FrameSet and invoke its \
   astClear method. The protected astClear#attribute method is notused \
   since we want the current Frame of the encapsulated FrameSet to be \
   remapped if required. */ \
   astClear( this->frameset, buf ); \
}

/*
*  Name:
*     MAKE_GET

*  Purpose:
*     Define a function to get an attribute value for a Region.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_GET(attribute,type)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static <type> Get<Attribute>( AstFrame *this )
*
*     that gets the value of a specified attribute for the encapsulated 
*     FrameSet of a Region (this). This function is intended to over-ride 
*     the astGet<Attribute> method inherited from the Frame class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*     type
*        The C type of the attribute.
*/

/* Define the macro. */
#define MAKE_GET(attribute,type) \
static type Get##attribute( AstFrame *this_frame ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to the Region structure */ \
   type result;                /* Value to return */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return (type) 0; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Obtain a pointer to the encapsulated FrameSet and invoke its \
   astGet<Attribute> method.  */ \
   result = astGet##attribute( this->frameset ); \
\
/* If an error occurred, clear the result value. */ \
   if ( !astOK ) result = (type) 0; \
\
/* Return the result. */ \
   return result; \
}

/*
*  Name:
*     MAKE_GET_AXIS

*  Purpose:
*     Define a function to get an attribute value for a Region axis.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_GET_AXIS(attribute,type)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static <type> Get<Attribute>( AstFrame *this, int axis )
*
*     that gets the value of a specified attribute for an axis of the
*     encapsulated FrameSet within a Region (this). This function is intended 
*     to over-ride the astGet<Attribute> method inherited from the Frame
*     class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*     type
*        The C type of the attribute.
*/

/* Define the macro. */
#define MAKE_GET_AXIS(attribute,type) \
static type Get##attribute( AstFrame *this_frame, int axis ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to the Region structure */ \
   type result;                /* Value to return */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return (type) 0; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Validate the axis index supplied. */ \
   (void) astValidateAxis( this, axis, "astGet" #attribute ); \
\
/* Obtain a pointer to the Region's encapsulated FrameSet and invoke its \
   astGet<Attribute> method.  */ \
   result = astGet##attribute( this->frameset, axis ); \
\
/* If an error occurred, clear the result value. */ \
   if ( !astOK ) result = (type) 0; \
\
/* Return the result. */ \
   return result; \
}

/*
*  Name:
*     MAKE_SET_SYSTEM

*  Purpose:
*     Define a function to set a System attribute value for a Region.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_SET_SYSTEM(attribute)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static void Set<Attribute>( AstFrame *this, AstSystemType value )
*
*     that sets the value of a specified attribute for the encapsulated 
*     FrameSet of a Region (this). This function is intended to over-ride the
*     astSet<Attribute> method inherited from the Frame class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*/

/* Define the macro. */
#define MAKE_SET_SYSTEM(attribute) \
static void Set##attribute( AstFrame *this_frame, AstSystemType value ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to the Region structure */ \
   const char *text;           /* Pointer to system string */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Convert the supplied value to a string using the astSystemString
   method of the current Frame in the encapsulated FrameSet. */ \
   text = astSystemString( this->frameset, value ); \
\
/* Set the value by invoking the public astSetC method on the encapusulated \
   FrameSet. This ensures that the current Frame of the encapsulated \
   FrameSet is re-mapped if necessary. */ \
   astSetC( this->frameset, #attribute, text ); \
}

/*
*  Name:
*     MAKE_SET

*  Purpose:
*     Define a function to set an attribute value for a Region.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_SET(attribute,type,x)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static void Set<Attribute>( AstFrame *this, <type> value )
*
*     that sets the value of a specified attribute for the encapsulated 
*     FrameSet of a Region (this). This function is intended to over-ride the
*     astSet<Attribute> method inherited from the Frame class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*     type
*        The C type of the attribute.
*     x
*        The single character code for the astSetx function for the given C 
*        type.
*/

/* Define the macro. */
#define MAKE_SET(attribute,type,x) \
static void Set##attribute( AstFrame *this_frame, type value ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to the Region structure */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Set the value by invoking the public astSetx method on the encapusulated \
   FrameSet. This ensures that the current Frame of the encapsulated \
   FrameSet is re-mapped if necessary. */ \
   astSet##x( this->frameset, #attribute, value ); \
}

/*
*  Name:
*     MAKE_SET_AXIS

*  Purpose:
*     Define a function to set an attribute value for a Region axis.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_SET_AXIS(attribute,type,x)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static void Set<Attribute>( AstFrame *this, int axis, <type> value )
*
*     that sets the value of a specified attribute for an axis of the
*     encapsulated FrameSet within a Region (this). This function is intended 
*     to over-ride the astSet<Attribute> method inherited from the Frame
*     class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*     type
*        The C type of the attribute.
*     x
*        The single character code for the astSetx function for the given C 
*        type.
*/

/* Define the macro. */
#define MAKE_SET_AXIS(attribute,type,x) \
static void Set##attribute( AstFrame *this_frame, int axis, type value ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to the Region structure */ \
   char buf[100];              /* Buffer for attribute name */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Validate the axis index supplied. */ \
   (void) astValidateAxis( this, axis, "astSet" #attribute ); \
\
/* We use the public astSetx method rather than the protected \
   astSet#attribute method so that the current Frame in the encapsulated \
   FrameSet will be re-mapped if necessary. Construct the attribute name. */ \
   sprintf( buf, "%s(%d)", #attribute, axis + 1 ); \
\
/* Obtain a pointer to the Region's encapsulated FrameSet and invoke its \
   astSet<x> method.  */ \
   astSet##x( this->frameset, buf, value ); \
}

/*
*  Name:
*     MAKE_TEST

*  Purpose:
*     Define a function to test if an attribute value is set for a Region.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_TEST(attribute)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static int Test<Attribute>( AstFrame *this )
*
*     that returns a boolean result (0 or 1) to indicate if the value
*     of a specified attribute for the encapsulated FrameSet within a
*     Region (this) is set. This function is intended to over-ride the
*     astTest<Attribute> method inherited from the Frame class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*/

/* Define the macro. */
#define MAKE_TEST(attribute) \
static int Test##attribute( AstFrame *this_frame ) { \
\
/* Local Variables: */ \
   AstRegion *this;            /* Pointer to Region structure */ \
   int result;                 /* Result to return */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return 0; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Obtain a pointer to the Region's encapsulated FrameSet and invoke its \
   astTest<Attribute> method.  */ \
   result = astTest##attribute( this->frameset ); \
\
/* If an error occurred, clear the result value. */ \
   if ( !astOK ) result = 0; \
\
/* Return the result. */ \
   return result; \
}

/*
*  Name:
*     MAKE_TEST_AXIS

*  Purpose:
*     Define a function to test if an attribute value is set for a Region
*     axis.

*  Type:
*     Private macro.

*  Synopsis:
*     #include "region.h"
*     MAKE_TEST_AXIS(attribute)

*  Class Membership:
*     Defined by the Region class.

*  Description:
*     This macro expands to an implementation of a private member function
*     of the form:
*
*        static int Test<Attribute>( AstFrame *this, int axis )
*
*     that returns a boolean result (0 or 1) to indicate if the value
*     of a specified attribute for an axis of the encapsulated FrameSet 
*     within a Region (this) is set. This function is intended to over-ride
*     the astTest<Attribute> method inherited from the Frame class.

*  Parameters:
*     attribute
*        Name of the attribute, as it appears in the function name.
*/

/* Define the macro. */
#define MAKE_TEST_AXIS(attribute) \
static int Test##attribute( AstFrame *this_frame, int axis ) { \
\
/* Local Variables: */ \
   AstRegion *this;              /* Pointer to the Region structure */ \
   int result;                   /* Value to return */ \
\
/* Check the global error status. */ \
   if ( !astOK ) return 0; \
\
/* Obtain a pointer to the Region structure. */ \
   this = (AstRegion *) this_frame; \
\
/* Validate the axis index supplied. */ \
   (void) astValidateAxis( this, axis, "astTest" #attribute ); \
\
/* Obtain a pointer to the Region's encapsulated FrameSet and invoke its \
   astTest<Attribute> method.  */ \
   result = astTest##attribute( this->frameset, axis ); \
\
/* If an error occurred, clear the result value. */ \
   if ( !astOK ) result = 0; \
\
/* Return the result. */ \
   return result; \
}

/* Header files. */
/* ============= */
/* Interface definitions. */
/* ---------------------- */
#include "error.h"               /* Error reporting facilities */
#include "memory.h"              /* Memory allocation facilities */
#include "object.h"              /* Base Object class */
#include "mapping.h"             /* Coordinate Mappings */
#include "unitmap.h"             /* Unit Mappings */
#include "permmap.h"             /* Coordinate permutation Mappings */
#include "cmpmap.h"              /* Compound Mappings */
#include "frame.h"               /* Parent Frame class */
#include "region.h"              /* Interface definition for this class */
#include "circle.h"              /* Circular regions */
#include "box.h"                 /* Box regions */
#include "cmpregion.h"           /* Compound regions */
#include "ellipse.h"             /* Elliptical regions */

/* Error code definitions. */
/* ----------------------- */
#include "ast_err.h"             /* AST error codes */

/* C header files. */
/* --------------- */
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Module Variables. */
/* ================= */
/* Define the class virtual function table and its initialisation flag as
   static variables. */
static AstRegionVtab class_vtab; /* Virtual function table */
static int class_init = 0;       /* Virtual function table initialised? */

/* Pointers to parent class methods which are extended by this class. */
static int (* parent_getnaxes)( AstFrame * );

/* Prototypes for Private Member Functions. */
/* ======================================== */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
static int MaskLD( AstRegion *, AstMapping *, int, int, const int[], const int ubnd[], long double [], long double );
#endif
static int MaskB( AstRegion *, AstMapping *, int, int, const int[], const int[], signed char[], signed char );
static int MaskD( AstRegion *, AstMapping *, int, int, const int[], const int[], double[], double );
static int MaskF( AstRegion *, AstMapping *, int, int, const int[], const int[], float[], float );
static int MaskI( AstRegion *, AstMapping *, int, int, const int[], const int[], int[], int );
static int MaskL( AstRegion *, AstMapping *, int, int, const int[], const int[], long int[], long int );
static int MaskS( AstRegion *, AstMapping *, int, int, const int[], const int[], short int[], short int );
static int MaskUB( AstRegion *, AstMapping *, int, int, const int[], const int[], unsigned char[], unsigned char );
static int MaskUI( AstRegion *, AstMapping *, int, int, const int[], const int[], unsigned int[], unsigned int );
static int MaskUL( AstRegion *, AstMapping *, int, int, const int[], const int[], unsigned long int[], unsigned long int );
static int MaskUS( AstRegion *, AstMapping *, int, int, const int[], const int[], unsigned short int[], unsigned short int );

static AstAxis *GetAxis( AstFrame *, int );
static AstFrame *GetRegionFrame( AstRegion * );
static AstFrame *PickAxes( AstFrame *, int, const int[], AstMapping ** );
static AstFrameSet *Conv( AstFrameSet *, AstFrameSet * );
static AstFrameSet *Convert( AstFrame *, AstFrame *, const char * );
static AstFrameSet *ConvertX( AstFrame *, AstFrame *, const char * );
static AstFrameSet *FindFrame( AstFrame *, AstFrame *, const char * );
static AstFrame *RegFrame( AstRegion *this );
static AstMapping *RegMapping( AstRegion *this );
static AstMapping *Simplify( AstMapping * );
static AstPointSet *RegBaseMesh( AstRegion * );
static AstPointSet *BndBaseMesh( AstRegion *, double *, double * );
static AstPointSet *RegMesh( AstRegion * );
static AstPointSet *RegTransform( AstRegion *, AstPointSet *, int, AstPointSet *, AstFrame ** );
static AstPointSet *ResolvePoints( AstFrame *, const double [], const double [], AstPointSet *, AstPointSet * );
static AstRegion *MapRegion( AstRegion *, AstMapping *, AstFrame * );
static AstSystemType SystemCode( AstFrame *, const char * );
static AstSystemType ValidateSystem( AstFrame *, AstSystemType, const char * );
static const char *Abbrev( AstFrame *, int, const char *, const char *, const char * );
static const char *Format( AstFrame *, int, double );
static const char *SystemString( AstFrame *, AstSystemType );
static const int *GetPerm( AstFrame * );
static double *RegCentre( AstRegion *this, double *, double **, int, int );
static double Angle( AstFrame *, const double[], const double[], const double[] );
static double AxAngle( AstFrame *, const double[], const double[], int );
static double AxDistance( AstFrame *, int, double, double );
static double AxOffset( AstFrame *, int, double, double );
static double Distance( AstFrame *, const double[], const double[] );
static double Gap( AstFrame *, int, double, int * );
static double Offset2( AstFrame *, const double[2], double, double, double[2] );
static int DumpUnc( AstRegion * );
static int TestUnc( AstRegion * );
static int Equal( AstObject *, AstObject * );
static int GetNaxes( AstFrame * );
static int IsUnitFrame( AstFrame * );
static int Match( AstFrame *, AstFrame *, int **, int **, AstMapping **, AstFrame ** );
static int Overlap( AstRegion *, AstRegion * );
static int OverlapX( AstRegion *, AstRegion * );
static int RegPins( AstRegion *, AstPointSet *, AstRegion *, int ** );
static int SubFrame( AstFrame *, AstFrame *, int, const int *, const int *, AstMapping **, AstFrame ** );
static int Unformat( AstFrame *, int, const char *, double * );
static int ValidateAxis( AstFrame *, int, const char * );
static void CheckPerm( AstFrame *, const int *, const char * );
static void Copy( const AstObject *, AstObject * );
static void Delete( AstObject * );
static void Dump( AstObject *, AstChannel * );
static void Negate( AstRegion * );
static void Norm( AstFrame *, double[] );
static void Offset( AstFrame *, const double[], const double[], double, double[] );
static void Overlay( AstFrame *, const int *, AstFrame * );
static void PermAxes( AstFrame *, const int[] );
static void RegBaseBox( AstRegion *this, double *, double * );
static void RegCurBox( AstRegion *this, double *, double * );
static void RegOverlay( AstRegion *, AstRegion * );
static void ReportPoints( AstMapping *, int, AstPointSet *, AstPointSet * );
static void Resolve( AstFrame *, const double [], const double [], const double [], double [], double *, double * );
static void SetAxis( AstFrame *, int, AstAxis * );
static void ValidateAxisSelection( AstFrame *, int, const int *, const char * );
static void SetRegFS( AstRegion *, AstFrame * );

static int GetBounded( AstRegion * );
static int DumpUnc( AstRegion * );
static AstRegion *GetDefUnc( AstRegion * );

static AstRegion *GetUnc( AstRegion *, int );
static int TestUnc( AstRegion * );
static void ClearUnc( AstRegion * );
static void SetUnc( AstRegion *, AstRegion * );

static const char *GetDomain( AstFrame * );
static int TestDomain( AstFrame * );
static void ClearDomain( AstFrame * );
static void SetDomain( AstFrame *, const char * );

static const char *GetFormat( AstFrame *, int );
static int TestFormat( AstFrame *, int );
static void ClearFormat( AstFrame *, int );
static void SetFormat( AstFrame *, int, const char * );

static const char *GetLabel( AstFrame *, int );
static int TestLabel( AstFrame *, int );
static void ClearLabel( AstFrame *, int );
static void SetLabel( AstFrame *, int, const char * );

static const char *GetSymbol( AstFrame *, int );
static int TestSymbol( AstFrame *, int );
static void ClearSymbol( AstFrame *, int );
static void SetSymbol( AstFrame *, int, const char * );

static const char *GetTitle( AstFrame * );
static void SetTitle( AstFrame *, const char * );
static void ClearTitle( AstFrame * );
static int TestTitle( AstFrame * );

static const char *GetUnit( AstFrame *, int );
static int TestUnit( AstFrame *, int );
static void ClearUnit( AstFrame *, int );
static void SetUnit( AstFrame *, int, const char * );

static int GetDigits( AstFrame * );
static int TestDigits( AstFrame * );
static void ClearDigits( AstFrame * );
static void SetDigits( AstFrame *, int );

static int GetDirection( AstFrame *, int );
static int TestDirection( AstFrame *, int );
static void ClearDirection( AstFrame *, int );
static void SetDirection( AstFrame *, int, int );

static int GetActiveUnit( AstFrame * );
static int TestActiveUnit( AstFrame * );
static void SetActiveUnit( AstFrame *, int );

static int GetMatchEnd( AstFrame * );
static int TestMatchEnd( AstFrame * );
static void ClearMatchEnd( AstFrame * );
static void SetMatchEnd( AstFrame *, int );

static int GetMaxAxes( AstFrame * );
static int TestMaxAxes( AstFrame * );
static void ClearMaxAxes( AstFrame * );
static void SetMaxAxes( AstFrame *, int );

static int GetMinAxes( AstFrame * );
static int TestMinAxes( AstFrame * );
static void ClearMinAxes( AstFrame * );
static void SetMinAxes( AstFrame *, int );

static int GetPermute( AstFrame * );
static int TestPermute( AstFrame * );
static void ClearPermute( AstFrame * );
static void SetPermute( AstFrame *, int );

static int GetPreserveAxes( AstFrame * );
static int TestPreserveAxes( AstFrame * );
static void ClearPreserveAxes( AstFrame * );
static void SetPreserveAxes( AstFrame *, int );

static double GetBottom( AstFrame *, int );
static int TestBottom( AstFrame *, int );
static void ClearBottom( AstFrame *, int );
static void SetBottom( AstFrame *, int, double );

static double GetTop( AstFrame *, int );
static int TestTop( AstFrame *, int );
static void ClearTop( AstFrame *, int );
static void SetTop( AstFrame *, int, double );

static double GetEpoch( AstFrame * );
static int TestEpoch( AstFrame * );
static void ClearEpoch( AstFrame * );
static void SetEpoch( AstFrame *, double );

static AstSystemType GetSystem( AstFrame * );
static int TestSystem( AstFrame * );
static void ClearSystem( AstFrame * );
static void SetSystem( AstFrame *, AstSystemType );

static AstSystemType GetAlignSystem( AstFrame * );
static int TestAlignSystem( AstFrame * );
static void ClearAlignSystem( AstFrame * );
static void SetAlignSystem( AstFrame *, AstSystemType );

static const char *GetAttrib( AstObject *, const char * );
static int TestAttrib( AstObject *, const char * );
static void ClearAttrib( AstObject *, const char * );
static void SetAttrib( AstObject *, const char * );

static int GetNegated( AstRegion * );
static int TestNegated( AstRegion * );
static void ClearNegated( AstRegion * );
static void SetNegated( AstRegion *, int );

static int GetClosed( AstRegion * );
static int TestClosed( AstRegion * );
static void ClearClosed( AstRegion * );
static void SetClosed( AstRegion *, int );

static int GetMeshSize( AstRegion * );
static int TestMeshSize( AstRegion * );
static void ClearMeshSize( AstRegion * );
static void SetMeshSize( AstRegion *, int );

static double GetFillFactor( AstRegion * );
static int TestFillFactor( AstRegion * );
static void ClearFillFactor( AstRegion * );
static void SetFillFactor( AstRegion *, double );

static int GetRegionFS( AstRegion * );
static int TestRegionFS( AstRegion * );
static void ClearRegionFS( AstRegion * );
static void SetRegionFS( AstRegion *, int );


/* Member functions. */
/* ================= */

static const char *Abbrev( AstFrame *this_frame, int axis, const char *fmt,
                           const char *str1, const char *str2 ) {
/*
*  Name:
*     Abbrev

*  Purpose:
*     Abbreviate a formatted Region axis value by skipping leading fields.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     const char *Abbrev( AstFrame *this, int axis, const char *fmt,
*                         const char *str1, const char *str2 )

*  Class Membership:
*     Region member function (over-rides the protected astAbbrev
*     method inherited from the Frame class).

*  Description:
*     This function compares two Region axis values that have been
*     formatted (using astFormat) and determines if they have any
*     redundant leading fields (i.e. leading fields in common which
*     can be suppressed when tabulating the values or plotting them on
*     the axis of a graph).

*  Parameters:
*     this
*        Pointer to the Region
*     axis
*        The number of the Region axis for which the values have
*        been formatted (axis numbering starts at zero for the first
*        axis).
*     fmt
*        Pointer to a constant null-terminated string containing the
*        format specification used to format the two values.
*     str1
*        Pointer to a constant null-terminated string containing the
*        first formatted value.
*     str1
*        Pointer to a constant null-terminated string containing the
*        second formatted value.

*  Returned Value:
*     A pointer into the "str2" string which locates the first
*     character in the first field that differs between the two
*     formatted values.
*
*     If the two values have no leading fields in common, the returned
*     value will point at the start of string "str2". If the two
*     values are equal, it will point at the terminating null at the
*     end of this string.

*  Notes:
*     - This function assumes that the format specification used was
*     the same when both values were formatted and that they both
*     apply to the same Region axis.
*     - A pointer to the start of "str2" will be returned if this
*     function is invoked with the global error status set, or if it
*     should fail for any reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   const char *result;           /* Pointer value to return */

/* Check the global error status. */
   if ( !astOK ) return str2;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis, "astAbbrev" );

/* Obtain a pointer to the Region's current Frame and invoke this
   Frame's astAbbrev method to perform the processing. Annul the Frame
   pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astAbbrev( fr, axis, fmt, str1, str2 );
   fr = astAnnul( fr );

/* If an error occurred, clear the result. */
   if ( !astOK ) result = str2;

/* Return the result. */
   return result;
}

static double Angle( AstFrame *this_frame, const double a[],
                     const double b[], const double c[] ) {
/*
*  Name:
*     Angle

*  Purpose:
*     Calculate the angle subtended by two points at a third point.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     double Angle( AstFrame *this, const double a[], const double b[],
*                   const double c[] )

*  Class Membership:
*     Region member function (over-rides the protected astAngle
*     method inherited from the Frame class).

*  Description:
*     This function finds the angle at point B between the line joining points 
*     A and B, and the line joining points C and B. These lines will in fact be
*     geodesic curves appropriate to the Frame in use. For instance, in
*     SkyFrame, they will be great circles.

*  Parameters:
*     this
*        Pointer to the Frame.
*     a
*        An array of double, with one element for each Frame axis
*        (Naxes attribute) containing the coordinates of the first point.
*     b
*        An array of double, with one element for each Frame axis
*        (Naxes attribute) containing the coordinates of the second point.
*     c
*        An array of double, with one element for each Frame axis
*        (Naxes attribute) containing the coordinates of the third point.

*  Returned Value:
*     astAngle
*        The angle in radians, from the line AB to the line CB. If the
*        Frame is 2-dimensional, it will be in the range $\pm \pi$,
*        and positive rotation is in the same sense as rotation from
*        the positive direction of axis 2 to the positive direction of
*        axis 1. If the Frame has more than 2 axes, a positive value will
*        always be returned in the range zero to $\pi$.

*  Notes:
*     - A value of AST__BAD will also be returned if points A and B are
*     co-incident, or if points B and C are co-incident.
*     - A value of AST__BAD will also be returned if this function is
*     invoked with the AST error status set, or if it should fail for
*     any reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double result;                /* Value to return */

/* Check the global error status. */
   if ( !astOK ) return AST__BAD;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke this
   Frame's astAngle method. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astAngle( fr, a, b, c );
   fr = astAnnul( fr );

/* If an error occurred, clear the result. */
   if ( !astOK ) result = AST__BAD;

/* Return the result. */
   return result;
}

static double AxAngle( AstFrame *this_frame, const double a[], const double b[], int axis ) {
/*
*  Name:
*     AxAngle

*  Purpose:
*     Returns the angle from an axis, to a line through two points.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     double AxAngle( AstFrame *this, const double a[], const double b[], int axis )

*  Class Membership:
*     Region member function (over-rides the protected astAxAngle
*     method inherited from the Frame class).

*  Description:
*     This function finds the angle, as seen from point A, between the positive
*     direction of a specified axis, and the geodesic curve joining point
*     A to point B.

*  Parameters:
*     this
*        Pointer to the Frame.
*     a
*        An array of double, with one element for each Frame axis
*        (Naxes attribute) containing the coordinates of the first point.
*     b
*        An array of double, with one element for each Frame axis
*        (Naxes attribute) containing the coordinates of the second point.
*     axis
*        The number of the Frame axis from which the angle is to be
*        measured (one-based)

*  Returned Value:
*        The angle in radians, from the positive direction of the
*        specified axis, to the line AB. If the Frame is 2-dimensional,
*        it will be in the range $\pm \pi$, and positive rotation is in
*        the same sense as rotation from the positive direction of axis 2
*        to the positive direction of axis 1. If the Frame has more than 2
*        axes, a positive value will always be returned in the range zero
*        to $\pi$.

*  Notes:
*     - The geodesic curve used by this function is the path of
*     shortest distance between two points, as defined by the
*     astDistance function.
*     - This function will return "bad" coordinate values (AST__BAD)
*     if any of the input coordinates has this value, or if the require
*     position angle is undefined.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double result;                /* Value to return */

/* Check the global error status. */
   if ( !astOK ) return AST__BAD;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis - 1, "astAxAngle" );

/* Obtain a pointer to the Region's encapsulated Frame and invoke the
   astAxAngle method for this Frame. Annul the Frame pointer
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astAxAngle( fr, a, b, axis );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = AST__BAD;

/* Return the result. */
   return result;
}

static double AxDistance( AstFrame *this_frame, int axis, double v1, double v2 ) {
/*
*  Name:
*     AxDistance

*  Purpose:
*     Find the distance between two axis values.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     double AxDistance( AstFrame *this, int axis, double v1, double v2 )

*  Class Membership:
*     Region member function (over-rides the protected astAxDistance
*     method inherited from the Frame class).

*  Description:
*     This function returns a signed value representing the axis increment
*     from axis value v1 to axis value v2.
*
*     For a simple Frame, this is a trivial operation returning the
*     difference between the two axis values. But for other derived classes
*     of Frame (such as a SkyFrame) this is not the case.

*  Parameters:
*     this
*        Pointer to the Frame.
*     axis
*        The index of the axis to which the supplied values refer. The
*        first axis has index 1.
*     v1
*        The first axis value.
*     v2
*        The second axis value.

*  Returned Value:
*     The distance between the two axis values.

*  Notes:
*     - This function will return a "bad" result value (AST__BAD) if
*     any of the input vaues has this value.
*     - A "bad" value will also be returned if this function is
*     invoked with the AST error status set, or if it should fail for
*     any reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double result;                /* Value to return */

/* Check the global error status. */
   if ( !astOK ) return AST__BAD;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis - 1, "astAxDistance" );

/* Obtain a pointer to the Region's encapsulated Frame and invoke the
   astAxDistance method for this Frame. Annul the Frame pointer
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astAxDistance( fr, axis, v1, v2 );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = AST__BAD;

/* Return the result. */
   return result;
}

static double AxOffset( AstFrame *this_frame, int axis, double v1, double dist ) {
/*
*  Name:
*     AxOffset

*  Purpose:
*     Add an increment onto a supplied axis value.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     double AxOffset( AstFrame *this, int axis, double v1, double dist )

*  Class Membership:
*     Region member function (over-rides the protected astAxOffset
*     method inherited from the Frame class).

*  Description:
*     This function returns an axis value formed by adding a signed axis
*     increment onto a supplied axis value.
*
*     For a simple Frame, this is a trivial operation returning the
*     sum of the two supplied values. But for other derived classes
*     of Frame (such as a SkyFrame) this is not the case.

*  Parameters:
*     this
*        Pointer to the Frame.
*     axis
*        The index of the axis to which the supplied values refer. The
*        first axis has index 1.
*     v1
*        The original axis value.
*     dist
*        The axis increment to add to the original axis value.

*  Returned Value:
*     The incremented axis value.

*  Notes:
*     - This function will return a "bad" result value (AST__BAD) if
*     any of the input vaues has this value.
*     - A "bad" value will also be returned if this function is
*     invoked with the AST error status set, or if it should fail for
*     any reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double result;                /* Value to return */

/* Check the global error status. */
   if ( !astOK ) return AST__BAD;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis - 1, "astAxOffset" );

/* Obtain a pointer to the Region's encapsulated Frame and invoke the
   astAxOffset method for this Frame. Annul the Frame pointer
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astAxOffset( fr, axis, v1, dist );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = AST__BAD;

/* Return the result. */
   return result;
}

static AstPointSet *BndBaseMesh( AstRegion *this, double *lbnd, double *ubnd ){
/*
*+
*  Name:
*     astBndBaseMesh

*  Purpose:
*     Return a PointSet containing points spread around part of the boundary 
*     of a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstPointSet *astBndBaseMesh( AstRegion *this, double *lbnd, double *ubnd )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a PointSet containing a set of points on the
*     boundary of the intersection between the supplied Region and the
*     supplied box. The points refer to the base Frame of the 
*     encapsulated FrameSet. If the boundary of the supplied Region does
*     not intersect the supplied box, then a PointSet containing a single 
*     bad point is returned.


*  Parameters:
*     this
*        Pointer to the Region.
*     lbnd
*        Pointer to an array holding the lower limits of the axis values 
*        within the required box.
*     ubnd
*        Pointer to an array holding the upper limits of the axis values 
*        within the required box.

*  Returned Value:
*     Pointer to the PointSet. The axis values in this PointSet will have 
*     associated accuracies derived from the uncertainties which were
*     supplied when the Region was created.
*     
*    If the Region does not intersect the supplied box, the returned
*    PointSet will contain a single point with a value of AST__BAD on
*    every axis.

*  Notes:
*    - A NULL pointer is returned if an error has already occurred, or if
*    this function should fail for any reason.
*-
*/

/* Local Variables: */
   AstBox *box;
   AstCmpRegion *cmpreg;
   AstPointSet *result;  
   double **ptr;    
   int ic;
   int nc;

/* Check the local error status. */
   if ( !astOK ) return NULL;

/* Form a Box describing the required box. */
   box = astBox( this, 1, lbnd, ubnd, NULL, "" );

/* Check there is partial overlap between the Regions.*/
   if( astOverlap( this, box ) > 3 ) {

/* Form a CmpRegion representing the intersection between the supplied
   Region and the above box. */
      cmpreg = astCmpRegion( this, box, AST__AND, "" );

/* Get the boundary mesh. */
      result = astRegBaseMesh( cmpreg );

/* Free resources. */
      cmpreg = astAnnul( cmpreg );

/* If the boundary of the supplied Region does not intersect the box,
   return a PointSet containing a single bad position. */
   } else {
      nc = astGetNin( this->frameset );
      result = astPointSet( 1, nc, "" );
      ptr = astGetPoints( result );
      if( astOK ) {
         for( ic = 0; ic < nc; ic++ ) ptr[ ic ][ 0 ] = AST__BAD;
      }
   }
      
/* Free resources. */
   box = astAnnul( box );

/* Return NULL if an error occurred. */
   if( !astOK ) result = astAnnul( result );

/* Return the required pointer. */
   return result;
}

static void CheckPerm( AstFrame *this_frame, const int *perm, const char *method ) {
/*
*  Name:
*     CheckPerm

*  Purpose:
*     Check that an array contains a valid permutation.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void CheckPerm( AstFrame *this, const int *perm, const char *method )

*  Class Membership:
*     Region member function (over-rides the protected astCheckPerm
*     method inherited from the Frame class).

*  Description:
*     This function checks the validity of a permutation array that
*     will be used to permute the order of a Frame's axes. If the
*     permutation specified by the array is not valid, an error is
*     reported and the global error status is set. Otherwise, the
*     function returns without further action.

*  Parameters:
*     this
*        Pointer to the Frame.
*     perm
*        Pointer to an array of integers with the same number of
*        elements as there are axes in the Frame. For each axis, the
*        corresponding integer gives the (zero based) axis index to be
*        used to identify the information for that axis (using the
*        un-permuted axis numbering). To be valid, the integers in
*        this array should therefore all lie in the range zero to
*        (naxes-1) inclusive, where "naxes" is the number of Frame
*        axes, and each value should occur exactly once.
*     method
*        Pointer to a constant null-terminated character string
*        containing the name of the method that invoked this function
*        to validate a permutation array. This method name is used
*        solely for constructing error messages.

*  Notes:
*     - Error messages issued by this function refer to the external
*     (public) numbering system used for axes (which is one-based),
*     whereas zero-based axis indices are used internally.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke this
   Frame's astCheckPerm method. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astCheckPerm( fr, perm, method );
   fr = astAnnul( fr );

}

static void ClearAttrib( AstObject *this_object, const char *attrib ) {
/*
*  Name:
*     ClearAttrib

*  Purpose:
*     Clear an attribute value for a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void ClearAttrib( AstObject *this, const char *attrib )

*  Class Membership:
*     Region member function (over-rides the astClearAttrib protected
*     method inherited from the Frame class).

*  Description:
*     This function clears the value of a specified attribute for a
*     Region, so that the default value will subsequently be used.

*  Parameters:
*     this
*        Pointer to the Region.
*     attrib
*        Pointer to a null terminated string specifying the attribute
*        name.  This should be in lower case with no surrounding white
*        space.
*/

/* Local Variables: */
   AstRegion *this;            /* Pointer to the Region structure */
   int len;                    /* Length of attrib string */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_object;

/* Obtain the length of the "attrib" string. */
   len = strlen( attrib );

/* Check the attribute name and clear the appropriate attribute. */

/* We first handle attributes that apply to the Region as a whole
   (rather than to the encapsulated FrameSet). */

/* Negated */
/* ------- */
   if ( !strcmp( attrib, "negated" ) ) {
      astClearNegated( this );

/* Closed */
/* ------ */
   } else if ( !strcmp( attrib, "closed" ) ) {
      astClearClosed( this );

/* FillFactor */
/* ---------- */
   } else if ( !strcmp( attrib, "fillfactor" ) ) {
      astClearFillFactor( this );

/* MeshSize */
/* -------- */
   } else if ( !strcmp( attrib, "meshsize" ) ) {
      astClearMeshSize( this );

/* We now check for atttributes of superclasses which apply to the Region
   as a whole. We do not want to pass these on to the encapsulated FrameSet. */

/* ID. */
/* --- */
   } else if ( !strcmp( attrib, "id" ) ) {
      astClearID( this );

/* Ident. */
/* ------ */
   } else if ( !strcmp( attrib, "ident" ) ) {
      astClearIdent( this );

/* Invert. */
/* ------- */
   } else if ( !strcmp( attrib, "invert" ) ) {
      astClearInvert( this );

/* Report. */
/* ------- */
   } else if ( !strcmp( attrib, "report" ) ) {
      astClearReport( this );


/* If the name was not recognised, test if it matches any of the
   read-only attributes of this class (including those of all superclasses). 
   If it does, then report an error. */
   } else if ( !strcmp( attrib, "class" ) ||
               !strcmp( attrib, "nin" ) ||
               !strcmp( attrib, "nobject" ) ||
               !strcmp( attrib, "nout" ) ||
               !strcmp( attrib, "bounded" ) ||
               !strcmp( attrib, "refcount" ) ||
               !strcmp( attrib, "tranforward" ) ||
               !strcmp( attrib, "traninverse" ) ) {
      astError( AST__NOWRT, "astClear: Invalid attempt to clear the \"%s\" "
                "value for a %s.", attrib, astGetClass( this ) );
      astError( AST__NOWRT, "This is a read-only attribute." );

/* Pass unrecognised attributes on to the Region's encapsulated FrameSet for
   further interpretation. Do not pass on FrameSet attributes since we
   pretend to the outside world that the encapsulated FrameSet is actually a
   Frame. Use the public astClear method rather than the protected
   astClearAttrib method so that the current Frame in the encapsulated
   FrameSet will be re-mapped if the attribute changes require it. */
   } else if ( strcmp( attrib, "base" ) &&
               strcmp( attrib, "current" ) &&
               strcmp( attrib, "nframe" ) ) {
      astClear( this->frameset, attrib );
   }
}

static AstFrameSet *Conv( AstFrameSet *from, AstFrameSet *to ){
/*
*  Name:
*     Conv

*  Purpose:
*     Find Mapping between Frames

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstFrameSet *Conv( AstFrameSet *from, AstFrameSet *to );

*  Class Membership:
*     Region member function 

*  Description:
*     This function provides a convenient interface for astConvert.
*     It is like astConvert except it does not alter the base Frames of
*     the supplied FrameSets and does not require a Domain list.

*  Parameters:
*     from
*        Pointer to the source FrameSet.
*     to
*        Pointer to the source FrameSet.

*  Returned Value:
*        The conversion FrameSet (see astConvert).

*/

/* Local Variables: */
   AstFrameSet *result;        /* FrameSet to return */
   int from_base;              /* Index of original base Frame in "from" */
   int to_base;                /* Index of original base Frame in "to" */

/* Check the global error status. */
   if( !astOK ) return NULL;

/* Note the indicesof the base Frames in the FrameSets. */
   to_base = astGetBase( to );
   from_base = astGetBase( from );

/* Invoke astConvert. */
   result = astConvert( from, to, "" );

/* Re-instate original base Frames. */
   astSetBase( to, to_base );
   astSetBase( from, from_base );

/* Return the result. */
   return result;
}

static AstFrameSet *Convert( AstFrame *from, AstFrame *to,
                             const char *domainlist ) {
/*
*  Name:
*     Convert

*  Purpose:
*     Determine how to convert between two coordinate systems.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstFrameSet *Convert( AstFrame *from, AstFrame *to,
*                           const char *domainlist )

*  Class Membership:
*     Region member function (over-rides the public astConvert
*     method inherited fromm the Frame class).

*  Description:
*     This function compares two Regions and determines whether it
*     is possible to convert between the coordinate systems which
*     their current Frames represent. If conversion is possible, it
*     returns a FrameSet which describes the conversion and which may
*     be used (as a Mapping) to transform coordinate values in either
*     direction.

*  Parameters:
*     from
*        Pointer to a Region whose current Frame represents the
*        "source" coordinate system.  Note that the Base attribute of
*        the Region may be modified by this function.
*     to
*        Pointer to a Region whose current Frame represents the
*        "destination" coordinate system.  Note that the Base
*        attribute of the Region may be modified by this function.
*     domainlist
*        Pointer to a null-terminated character string containing a
*        comma-separated list of Frame domains. This may be used to
*        define a priority order for the different intermediate
*        coordinate systems that might be used to perform the
*        conversion.
*
*        The function will first try to obtain a conversion by making
*        use only of intermediate Frames whose Domain attribute
*        matches the first domain in this list. If this fails, the
*        second domain in the list will be used, and so on, until
*        conversion is achieved. A blank domain (e.g. two consecutive
*        commas) indicates that all Frames should be considered,
*        regardless of their Domain attributes. The list is
*        case-insensitive and all white space is ignored.

*  Returned Value:
*        If the requested coordinate conversion is possible, the
*        function returns a pointer to a FrameSet which describes the
*        conversion. Otherwise, a null Object pointer (AST__NULL) is
*        returned without error.
*
*        If a FrameSet is returned, it will contain two Frames. Frame
*        number 1 (its base Frame) will describe the source coordinate
*        system, corresponding to the "from" parameter. Frame number 2
*        (its current Frame) will describe the destination coordinate
*        system, corresponding to the "to" parameter. The Mapping
*        which inter-relates these Frames will perform the required
*        conversion between the two coordinate systems.

*  Notes:
*     - The returned FrameSet will not contain any Regions. If one or
*     more of the supplied Frames are in fact Regions, the corresponding 
*     Frames in any returned FrameSet will described the encapsulated
*     Frame, without any region information.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrameSet *result;       /* Returned FrameSet */

/* Check the inherited status. */
   if ( !astOK ) return NULL;

/* If the "from" pointer is a Region, get a pointer to the current Frame of
   the encapsulated FrameSet and use it instead of the supplied pointer. */
   if( astIsARegion( from ) ) {
      from = astGetFrame( ((AstRegion *) from)->frameset, AST__CURRENT );
   } else {
      from = astClone( from );
   }

/* If the "to" pointer is a Region, get a pointer to the current Frame of
   the encapsulated FrameSet and use it instead of the supplied pointer. */
   if( astIsARegion( to ) ) {
      to = astGetFrame( ((AstRegion *) to)->frameset, AST__CURRENT );
   } else {
      to = astClone( to );
   }
 
/* Now invoke astConvert on the above Frames. */
   result = astConvert( from, to, domainlist );

/* Annul the pointers used above. */
   from = astAnnul( from );
   to = astAnnul( to );

/* Return the result */
   return result;
}

static AstFrameSet *ConvertX( AstFrame *to, AstFrame *from,
                              const char *domainlist ) {
/*
*  Name:
*     astConvertX

*  Purpose:
*     Determine how to convert between two coordinate systems.

*  Type:
*     Region member function (over-rides the protected astConvertX
*     method inherited fromm the Frame class).

*  Synopsis:
*     #include "region.h"
*     AstFrameSet *astConvertX( AstFrame *to, AstFrame *from,
*                               const char *domainlist )

*  Class Membership:
*     Frame method.

*  Description:
*     This function performs the processing for the public astConvert
*     method and has exactly the same interface except that the order
*     of the first two arguments is swapped. This is a trick to allow
*     the astConvert method to be over-ridden by derived classes on
*     the basis of the class of either of its first two arguments.
*
*     See the astConvert method for details of the interface.
*-
*/

/* Local Variables: */
   AstFrameSet *result;       /* Returned FrameSet */

/* Check the inherited status. */
   if ( !astOK ) return NULL;

/* If the "to" pointer is a Region, get a pointer to the current Frame of
   the encapsulated FrameSet and use it instead of the supplied pointer. */
   if( astIsARegion( to ) ) {
      to = astGetFrame( ((AstRegion *) to)->frameset, AST__CURRENT );
   } else {
      to = astClone( to );
   }
 
/* If the "from" pointer is a Region, get a pointer to the current Frame of
   the encapsulated FrameSet and use it instead of the supplied pointer. */
   if( astIsARegion( from ) ) {
      from = astGetFrame( ((AstRegion *) from)->frameset, AST__CURRENT );
   } else {
      from = astClone( from );
   }

/* Now invoke astConvertX on the above Frames. */
   result = astConvertX( to, from, domainlist );

/* Annul the pointers used above. */
   from = astAnnul( from );
   to = astAnnul( to );

/* Return the result */
   return result;
}

static double Distance( AstFrame *this_frame, const double point1[], 
                        const double point2[] ) {
/*
*  Name:
*     Distance

*  Purpose:
*     Calculate the distance between two points.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     double Distance( AstFrame *this, const double point1[], 
*                      const double point2[] )

*  Class Membership:
*     Region member function (over-rides the protected astDistance
*     method inherited from the Frame class).

*  Description:
*     This function finds the distance between two points whose
*     Region coordinates are given. The distance calculated is that
*     along the geodesic curve that joins the two points.

*  Parameters:
*     this
*        Pointer to the Region.
*     point1
*        An array of double, with one element for each Region axis
*        containing the coordinates of the first point.
*     point2
*        An array of double, with one element for each Region axis
*        containing the coordinates of the second point.

*  Returned Value:
*     The distance between the two points.

*  Notes:
*     - This function will return a "bad" result value (AST__BAD) if
*     any of the input coordinates has this value.
*     - A "bad" value will also be returned if this function is
*     invoked with the AST error status set or if it should fail for
*     any reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double result;                /* Value to return */

/* Check the global error status. */
   if ( !astOK ) return AST__BAD;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's current Frame and invoke this
   Frame's astDistance method. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astDistance( fr, point1, point2 );
   fr = astAnnul( fr );

/* If an error occurred, clear the result. */
   if ( !astOK ) result = AST__BAD;

/* Return the result. */
   return result;
}

static int DumpUnc( AstRegion *this ) {
/*
*+
*  Name:
*     astDumpUnc

*  Purpose:
*     Should the uncertainty Region in a Region object be dumped?

*  Type:
*     Protected function.

*  Synopsis:
*     int astDumpUnc( AstRegion *this )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a flag indicating the uncertainty Region of the
*     supplied Region should be included when writing the Region out to a
*     Channel via the Dump function.

*  Parameters:
*     this
*        Pointer to the Region whose data are being written.

*  Returned Value:
*     Non-zero if the uncertainty Region should be included in the DUmp.
*     Zero otherwise.

*-
*/

/* Check the global error status. */
   if ( !astOK ) return 0;

/* Include the uncertainty Region if it is defined and is not a default
   uncertainty Region. */
   return ( this->unc && astTestUnc( this ) );
}

static int Equal( AstObject *this_object, AstObject *that_object ) {
/*
*  Name:
*     Equal

*  Purpose:
*     Test if two Objects are equivalent.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int Equal( AstObject *this_object, AstObject *that_object ) 

*  Class Membership:
*     Region member function (over-rides the astEqual protected
*     method inherited from the Frame class).

*  Description:
*     This function returns a boolean result (0 or 1) to indicate whether
*     two Regions are equivalent. 

*  Parameters:
*     this
*        Pointer to the first Region.
*     that
*        Pointer to the second Region.

*  Returned Value:
*     One if the Regions are equivalent, zero otherwise.

*  Notes:
*     - The Regions are equivalent if they are of the same class, have
*     equal PointSets, have equal base Frames, have equal current Frames,
*     and if the Mapping between base Frames is a UnitMap. In addition, the 
*     Negated attribute must have the same value in both Regions, as must 
*     the Closed attribute.
*     - A value of zero will be returned if this function is invoked
*     with the global status set, or if it should fail for any reason.
*/

/* Local Variables: */
   AstFrame *bf1;
   AstFrame *bf2;
   AstFrame *cf1;
   AstFrame *cf2;
   AstMapping *m1;
   AstMapping *m2;
   AstRegion *that;              
   AstRegion *this;              
   const char *class1;
   const char *class2;
   int result;                   

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Check that the two objects have the same class. */
   class1 = astGetClass( this_object );
   class2 = astGetClass( that_object );
   if( astOK && !strcmp( class1, class2 ) ) {

/* Obtain pointers to the two Region structures. */
      this = (AstRegion *) this_object;
      that = (AstRegion *) that_object;

/* Test their PointSets for equality. */
      if( astEqual( this->points, that->points ) ){

/* Test their base Frames for equality. */
         bf1 = astGetFrame( this->frameset, AST__BASE );
         bf2 = astGetFrame( that->frameset, AST__BASE );
         if( astEqual( bf1, bf2 ) ){

/* Test their current Frames for equality. */
            cf1 = astGetFrame( this->frameset, AST__CURRENT );
            cf2 = astGetFrame( that->frameset, AST__CURRENT );
            if( astEqual( cf1, cf2 ) ){

/* Get the two Mappings and check that they are equal */
               m1 = astGetMapping( this->frameset, AST__BASE, AST__CURRENT );
               m2 = astGetMapping( that->frameset, AST__BASE, AST__CURRENT );
               if( astEqual( m1, m2 ) ) {

/* Test the Negated and Closed flags are equal */
                  if( astGetNegated( this ) == astGetNegated( that ) &&
                       astGetClosed( this ) == astGetClosed( that ) ) result = 1;
               }

/* Free resources. */
               m1 = astAnnul( m1 );
               m2 = astAnnul( m2 );
            }

            cf1 = astAnnul( cf1 );
            cf2 = astAnnul( cf2 );
         }

         bf1 = astAnnul( bf1 );
         bf2 = astAnnul( bf2 );
      }
   }

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = 0;

/* Return the result, */
   return result;
}

static void ClearUnc( AstRegion *this ){
/*
*+
*  Name:
*     astClearUnc

*  Purpose:
*     Erase any uncertainty information in a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     void astClearUnc( AstRegion *this )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function erases all uncertainty information, whether default
*     or not, from a Region.

*  Parameters:
*     this
*        Pointer to the Region.

*-
*/

/* Check the inherited status. */
   if( !astOK ) return;

/* The base Region class stores a pointer to the uncertainty Region in
   the Region structure. */
   if( this->unc ) this->unc = astAnnul( this->unc );
   this->defunc = 1;

}

static AstFrameSet *FindFrame( AstFrame *target_frame, AstFrame *template,
                               const char *domainlist ) {
/*
*  Name:
*     FindFrame

*  Purpose:
*     Find a coordinate system with specified characteristics.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstFrameSet *FindFrame( AstFrame *target, AstFrame *template,
*                             const char *domainlist )

*  Class Membership:
*     Region member function (over-rides the astFindFrame method
*     inherited from the Frame class).

*  Description:
*     This function uses a "template" Frame to search a Region to
*     identify a coordinate system which has a specified set of
*     characteristics. If a suitable coordinate system can be found,
*     the function returns a pointer to a FrameSet which describes the
*     required coordinate system and how to convert coordinates to and
*     from it.

*  Parameters:
*     target
*        Pointer to the target Region.  
*     template
*        Pointer to the template Frame, which should be an instance of
*        the type of Frame you wish to find.
*     domainlist
*        Pointer to a null-terminated character string containing a
*        comma-separated list of Frame domains. This may be used to
*        establish a priority order for the different types of
*        coordinate system that might be found.
*
*        The function will first try to find a suitable coordinate
*        system whose Domain attribute equals the first domain in this
*        list. If this fails, the second domain in the list will be
*        used, and so on, until a result is obtained. A blank domain
*        (e.g. two consecutive commas) indicates that any coordinate
*        system is acceptable (subject to the template) regardless of
*        its domain.
*
*        This list is case-insensitive and all white space is ignored.
*        If you do not wish to restrict the domain in this way, you
*        should supply an empty string.

*  Returned Value:
*     If the search is successful, the function returns a pointer to a
*     FrameSet which contains the Frame found and a description of how
*     to convert to (and from) the coordinate system it
*     represents. Otherwise, a null Object pointer (AST__NULL) is
*     returned without error.
*
*     If a FrameSet is returned, it will contain two Frames. Frame
*     number 1 (its base Frame) represents the target coordinate
*     system and will be the same as the target. Frame number 2 (its 
*     current Frame) will be a Frame representing the coordinate system 
*     which the function found. The Mapping which inter-relates these two 
*     Frames will describe how to convert between their respective coordinate 
*     systems. Note, the Frames in this FrameSet will not be Regions -
*     that is, they will be simple Frames or other derived classes.

*  Notes:
*     - A null Object pointer (AST__NULL) will be returned if this
*     function is invoked with the AST error status set, or if it
*     should fail for any reason.
*/

/* Local Variables: */
   AstFrameSet *result;          /* Pointer to result FrameSet */
   AstFrame *fr;                 /* Pointer to encapsulated Frame */

/* Initialise. */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Invoke the astFindFrame method on the current Frame of the
   encapsulated FrameSet within the target Region. */
   fr = astGetFrame( ((AstRegion *) target_frame)->frameset, AST__CURRENT );
   result = astFindFrame( fr, template, domainlist );
   fr = astAnnul( fr );

/* Return the result. */
   return result;
}

static const char *Format( AstFrame *this_frame, int axis, double value ) {
/*
*  Name:
*     Format

*  Purpose:
*     Format a coordinate value for a Region axis.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     const char *Format( AstFrame *this, int axis, double value )

*  Class Membership:
*     Region member function (over-rides the astFormat method
*     inherited from the Frame class).

*  Description:
*     This function returns a pointer to a string containing the
*     formatted (character) version of a coordinate value for a
*     Region axis. The formatting applied is that specified by a
*     previous invocation of the astSetFormat method. A suitable
*     default format is applied if necessary.

*  Parameters:
*     this
*        Pointer to the Region.
*     axis
*        The number of the axis (zero-based) for which formatting is
*        to be performed.
*     value
*        The coordinate value to be formatted.

*  Returned Value:
*     A pointer to a null-terminated string containing the formatted
*     value.

*  Notes:
*     - The returned string pointer may point at memory allocated
*     within the Region object, or at static memory. The contents of
*     the string may be over-written or the pointer may become invalid
*     following a further invocation of the same function or deletion
*     of the Region. A copy of the string should therefore be made
*     if necessary.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   const char *result;           /* Pointer value to return */

/* Check the global error status. */
   if ( !astOK ) return NULL;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis, "astFormat" );

/* Obtain a pointer to the Region's current Frame and invoke the
   astFormat method for this Frame. Annul the Frame pointer
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astFormat( fr, axis, value );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = NULL;

/* Return the result. */
   return result;
}

static double Gap( AstFrame *this_frame, int axis, double gap, int *ntick ) {
/*
*  Name:
*     Gap

*  Purpose:
*     Find a "nice" gap for tabulating Region axis values.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     double Gap( AstFrame *this, int axis, double gap, int *ntick )

*  Class Membership:
*     Region member function (over-rides the protected astGap method
*     inherited from the Frame class).

*  Description:
*     This function returns a gap size which produces a nicely spaced
*     series of formatted values for a Region axis, the returned gap
*     size being as close as possible to the supplied target gap
*     size. It also returns a convenient number of divisions into
*     which the gap can be divided.

*  Parameters:
*     this
*        Pointer to the Region.
*     axis
*        The number of the axis (zero-based) for which a gap is to be found.
*     gap
*        The target gap size.
*     ntick
*        Address of an int in which to return a convenient number of
*        divisions into which the gap can be divided.

*  Returned Value:
*     The nice gap size.

*  Notes:
*     - A value of zero is returned if the target gap size is zero.
*     - A negative gap size is returned if the supplied gap size is negative.
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double result;                /* Gap value to return */

/* Check the global error status. */
   if ( !astOK ) return 0.0;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis, "astGap" );

/* Obtain a pointer to the Region's current Frame and invoke this
   Frame's astGap method to obtain the required gap value. Annul the
   Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astGap( fr, axis, gap, ntick );
   fr = astAnnul( fr );

/* If an error occurred, clear the result. */
   if ( !astOK ) result = 0.0;

/* Return the result. */
   return result;
}

static const char *GetAttrib( AstObject *this_object, const char *attrib ) {
/*
*  Name:
*     GetAttrib

*  Purpose:
*     Get the value of a specified attribute for a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     const char *GetAttrib( AstObject *this, const char *attrib )

*  Class Membership:
*     Region member function (over-rides the protected astGetAttrib
*     method inherited from the Frame class).

*  Description:
*     This function returns a pointer to the value of a specified
*     attribute for a Region, formatted as a character string.

*  Parameters:
*     this
*        Pointer to the Region.
*     attrib
*        Pointer to a null-terminated string containing the name of
*        the attribute whose value is required. This name should be in
*        lower case, with all white space removed.

*  Returned Value:
*     - Pointer to a null-terminated string containing the attribute
*     value.

*  Notes:
*     - The returned string pointer may point at memory allocated
*     within the Region, or at static memory. The contents of the
*     string may be over-written or the pointer may become invalid
*     following a further invocation of the same function or any
*     modification of the Region. A copy of the string should
*     therefore be made if necessary.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Constants: */
#define BUFF_LEN 50              /* Max. characters in result buffer */

/* Local Variables: */
   AstRegion *this;              /* Pointer to the Region structure */
   const char *result;           /* Pointer value to return */
   double dval;                  /* Floating point attribute value */
   int ival;                     /* Integer attribute value */
   int len;                      /* Length of attrib string */
   static char buff[ BUFF_LEN + 1 ]; /* Buffer for string result */

/* Initialise. */
   result = NULL;

/* Check the global error status. */   
   if ( !astOK ) return result;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_object;

/* Obtain the length of the attrib string. */
   len = strlen( attrib );

/* Compare "attrib" with each recognised attribute name in turn,
   obtaining the value of the required attribute. If necessary, write
   the value into "buff" as a null-terminated string in an appropriate
   format.  Set "result" to point at the result string. */

/* We first handle attributes that apply to the Region as a whole
   (rather than to the encapsulated FrameSet). */

/* Negated */
/* ------- */
   if ( !strcmp( attrib, "negated" ) ) {
      ival = astGetNegated( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Closed */
/* ------ */
   } else if ( !strcmp( attrib, "closed" ) ) {
      ival = astGetClosed( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* FillFactor */
/* ---------- */
   } else if ( !strcmp( attrib, "fillfactor" ) ) {
      dval = astGetFillFactor( this );
      if ( astOK ) {
         (void) sprintf( buff, "%.*g", DBL_DIG, dval );
         result = buff;
      }

/* MeshSize */
/* -------- */
   } else if ( !strcmp( attrib, "meshsize" ) ) {
      ival = astGetMeshSize( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Bounded */
/* ------- */
   } else if ( !strcmp( attrib, "bounded" ) ) {
      ival = astGetBounded( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Now get the values of attributes inherited from parent classes. We do
   this to avoid the request being passed on to the encapsulated FrameSet
   below. */

/* Class. */
/* ------ */
   } else if ( !strcmp( attrib, "class" ) ) {
      result = astGetClass( this );

/* ID. */
/* --- */
   } else if ( !strcmp( attrib, "id" ) ) {
      result = astGetID( this );

/* Ident. */
/* ------ */
   } else if ( !strcmp( attrib, "ident" ) ) {
      result = astGetIdent( this );

/* Invert. */
/* ------- */
   } else if ( !strcmp( attrib, "invert" ) ) {
      ival = astGetInvert( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Nin. */
/* ---- */
   } else if ( !strcmp( attrib, "nin" ) ) {
      ival = astGetNin( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Nobject. */
/* -------- */
   } else if ( !strcmp( attrib, "nobject" ) ) {
      ival = astGetNobject( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Nout. */
/* ----- */
   } else if ( !strcmp( attrib, "nout" ) ) {
      ival = astGetNout( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* RefCount. */
/* --------- */
   } else if ( !strcmp( attrib, "refcount" ) ) {
      ival = astGetRefCount( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Report. */
/* ------- */
   } else if ( !strcmp( attrib, "report" ) ) {
      ival = astGetReport( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* TranForward. */
/* ------------ */
   } else if ( !strcmp( attrib, "tranforward" ) ) {
      ival = astGetTranForward( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* TranInverse. */
/* ------------ */
   } else if ( !strcmp( attrib, "traninverse" ) ) {
      ival = astGetTranInverse( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", ival );
         result = buff;
      }

/* Pass unrecognised attributes on to the Region's encapsulated FrameSet for
   further interpretation. Do not pass on FrameSet attributes since we
   pretend to the outside world that the encapsulated FrameSet is actually a
   Frame. */
   } else if ( strcmp( attrib, "base" ) &&
               strcmp( attrib, "current" ) &&
               strcmp( attrib, "nframe" ) ) {
      result = astGetAttrib( this->frameset, attrib );
   }

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = NULL;

/* Return the result. */
   return result;

/* Undefine macros local to this function. */
#undef BUFF_LEN
}

static int GetBounded( AstRegion *this ) {
/*
*+
*  Name:
*     astGetBounded

*  Purpose:
*     Is the Region bounded?

*  Type:
*     Protected function.

*  Synopsis:
*     int astGetBounded( AstRegion *this )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a flag indicating if the Region is bounded.
*     The implementation provided by the base Region class is suitable
*     for Region sub-classes representing the inside of a single closed 
*     curve (e.g. Circle, Ellipse, Box, etc). Other sub-classes (such as
*     CmpRegion, PointList, etc ) may need to provide their own
*     implementations.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     Non-zero if the Region is bounded. Zero otherwise.

*-
*/

/* For Regions which are defined by one or more clsoed curves such as Circles, 
   Boxes, etc, the Region is bounded so long as it has not been negated. 
   Classes for which this is not true should over-ride this implementation. */
   return !astGetNegated( this );
}

static AstAxis *GetAxis( AstFrame *this_frame, int axis ) {
/*
*  Name:
*     GetAxis

*  Purpose:
*     Obtain a pointer to a specified Axis from a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstAxis *GetAxis( AstFrame *this, int axis )

*  Class Membership:
*     Region member function (over-rides the astGetAxis method
*     inherited from the Frame class).

*  Description:
*     This function returns a pointer to the Axis object associated
*     with one of the axes of the current Frame of a Region. This
*     object describes the quantity which is represented along that
*     axis.

*  Parameters:
*     this
*        Pointer to the Region.
*     axis
*        The number of the axis (zero-based) for which an Axis pointer
*        is required.

*  Returned Value:
*     A pointer to the requested Axis object.

*  Notes:
*     - The reference count of the requested Axis object will be
*     incremented by one to reflect the additional pointer returned by
*     this function.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstAxis *result;              /* Pointer to Axis */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return NULL;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis, "astGetAxis" );

/* Obtain a pointer to the Region's encapsulated FrameSet and invoke 
   this FrameSet's astGetAxis method to obtain the required Axis
   pointer. */
   result = astGetAxis( this->frameset, axis );

/* If an error occurred, annul the result. */
   if ( !astOK ) result = astAnnul( result );

/* Return the result. */
   return result;
}

static AstRegion *GetDefUnc( AstRegion *this ) {
/*
*+
*  Name:
*     astGetDefUnc

*  Purpose:
*     Obtain a pointer to the default uncertainty Region for a given Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstRegion *astGetDefUnc( AstRegion *this ) 

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a pointer to a Region which represents the
*     default uncertainty associated with a position on the boundary of the 
*     given  Region. The returned Region refers to the base Frame within the 
*     FrameSet encapsulated by the supplied Region.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     A pointer to the Region. This should be annulled (using astAnnul)
*     when no longer needed.

*  Notes:
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   AstFrame *bfrm;            /* Base Frame of supplied Region */
   AstRegion *result;         /* Returned pointer */
   double *lbnd;              /* Ptr. to array holding axis lower bounds */
   double *ubnd;              /* Ptr. to array holding axis upper bounds */
   double axlen;              /* Total length of bounding box on one axis */
   int i;                     /* Axis index */
   int nax;                   /* Number of base Frame axes */

/* Initialise */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Get a pointer to the base Frame in the supplied Region. */
   bfrm = astGetFrame( this->frameset, AST__BASE );

/* Get the number of base Frame axes. */
   nax = astGetNaxes( bfrm );

/* Get the base frame bounding box of the supplied Region. */
   lbnd = astMalloc( sizeof( double)*(size_t) nax );
   ubnd = astMalloc( sizeof( double)*(size_t) nax );
   astRegBaseBox( this, lbnd, ubnd );

/* Create a Box covering 1.0E-6 of this bounding box, centred on the origin. */
   if( astOK ) {

      for( i = 0; i < nax; i++ ) {
         axlen = astAxDistance( bfrm, i + 1, lbnd[ i ], ubnd[ i ] );
         lbnd[ i ] = 0.0;
         ubnd[ i ] = 0.5E-6*axlen;
      }

      result = (AstRegion *) astBox( bfrm, 0, lbnd, ubnd, NULL, "" );

   }

/* Free resources. */
   lbnd = astFree( lbnd );
   ubnd = astFree( ubnd );
   bfrm = astAnnul( bfrm );

/* Return NULL if an error occurred. */
   if( !astOK ) result = astAnnul( result );

/* Return the required pointer. */
   return result;
}

static AstRegion *GetUnc( AstRegion *this, int ifrm ) {
/*
*+
*  Name:
*     astGetUnc

*  Purpose:
*     Obtain a pointer to the uncertainty Region for a given Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstRegion *astGetUnc( AstRegion *this, int ifrm ) 

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a pointer to a Region which represents the
*     uncertainty associated with a position on the boundary of the given 
*     Region. The returned Region can refer to the either the base or 
*     the current Frame within the FrameSet encapsulated by the supplied 
*     Region as specified by the "ifrm" parameter. If the returned Region is 
*     re-centred at some point on the boundary of the supplied Region, then 
*     the re-centred Region will represent the region in which the true 
*     boundary position could be.

*  Parameters:
*     this
*        Pointer to the Region.
*     ifrm
*        The index of a Frame within the FrameSet encapsulated by "this".
*        The returned Region will refer to the requested Frame. It should
*        be either AST__CURRENT or AST__BASE.

*  Returned Value:
*     A pointer to the Region. This should be annulled (using astAnnul)
*     when no longer needed.

*  Notes:
*     - A default uncertainty Region will be created if the supplied Region 
*     does not have an uncertainty Region.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   AstFrame *frm;             /* Current Frame from supplied Region */
   AstMapping *map;           /* Supplied to uncertainty Mapping */
   AstRegion *result;         /* Returned pointer */

/* Initialise */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* If the supplied Region has no uncertainty Region, create a default
   one and store it in the supplied Region structure. */
   if( !this->unc ) {
      this->unc = astGetDefUnc( this );

/* Prevent the Dump function from writing this uncertainty Region. */
      this->defunc = 1;
   }

/* If the uncertainty Region is the base Frame is required, just return a
   clone of the stored Region pointer. The Frame represented by an
   uncertainty Region will always (barring bugs!) be the base Frame of
   its parent Region. */
   if( ifrm == AST__BASE ) {
      result = astClone( this->unc );

/* If the uncertainty Region is the current Frame is required... */
   } else {

/* Get a Mapping from the Frame represented by the uncertainty Region
   (the Region base Frame) to the Region current Frame. */
      map = astGetMapping( this->frameset, AST__BASE, AST__CURRENT );

/* If this is a UnitMap, the uncertainty Region is already in the correct 
   Frame, so just return the stored pointer. */
      if( astIsAUnitMap( map ) ) {
         result = astClone( this->unc );

/* Otherwise, use this Mapping to map the uncertainty Region into the current
   Frame. */
      } else {
         frm = astGetFrame( this->frameset, AST__CURRENT );
         result = astMapRegion( this->unc, map, frm );

/* Free resources. */
         frm = astAnnul( frm );
      }

      map = astAnnul( map );
   }

/* Return NULL if an error occurred. */
   if( !astOK ) result = astAnnul( result );

/* Return the required pointer. */
   return result;
}

static int TestUnc( AstRegion *this ) {
/*
*+
*  Name:
*     astTestUnc

*  Purpose:
*     Does the Region contain non-default uncertainty information?

*  Type:
*     Protected function.

*  Synopsis:
*     int astTestUnc( AstRegion *this )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a flag indicating if the uncertainty Region in 
*     the supplied Region was supplied explicit (i.e. is not a default 
*     uncertainty Region).

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     Non-zero if the uncertainty Region was supplied explicitly.
*     Zero otherwise.

*-
*/

/* Check the global error status. */
   if ( !astOK ) return 0;

/* The base Region class stores a flag in the Region structure to
   indicate if the uncertainty Region is default or not. */
   return this->unc && !this->defunc;
}

static AstFrame *RegFrame( AstRegion *this ) {
/*
*+
*  Name:
*     astRegFrame

*  Purpose:
*     Obtain a pointer to the current Frame for a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstFrame *astRegFrame( AstRegion *this ) 

*  Class Membership:
*     Region virtual function 

*  Description:
*     This function returns a pointer to the current Frame in the encapsulated 
*     FrameSet. This is a clone, not a deep copy, of the pointer stored
*     in the FrameSet. For a deep copy, use astGetRegionFrame.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     A pointer to the Frame.

*  Notes:
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Check the global error status. */
   if ( !astOK ) return NULL;

/* Return the required pointer. */
   return astGetFrame( this->frameset, AST__CURRENT );
}

static AstMapping *RegMapping( AstRegion *this ) {
/*
*  Name:
*     RegMapping

*  Purpose:
*     Obtain a pointer to the simplified base->current Mapping for a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstMapping *RegMapping( AstRegion *this ) 

*  Class Membership:
*     Region member function 

*  Description:
*     This function returns a pointer to the Mapping from the base to the
*     current Frame int he encapsulated FrameSet. The returned Mapping is
*     simplified before being returned.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     A pointer to the Mapping.

*  Notes:
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstMapping *map;           /* Unsimplified Mapping */
   AstMapping *result;        /* Simplified Mapping */

/* Initialise */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Get the Mapping */
   map = astGetMapping( this->frameset, AST__BASE, AST__CURRENT );

/* Simplify it. */
   result = astSimplify( map );

/* Annul the pointer to the unsimplified Mapping */
   map = astAnnul( map );

/* Return the required pointer. */
   return result;
}

static int GetNaxes( AstFrame *this_frame ) {
/*
*  Name:
*     GetNaxes

*  Purpose:
*     Determine how many axes a Region has.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int GetNaxes( AstFrame *this )

*  Class Membership:
*     Region member function (over-rides the astGetNaxes method
*     inherited from the Frame class).

*  Description:
*     This function returns the number of axes for a Region. This is equal
*     to the number of axes in its current Frame.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     The number of Region axes (zero or more).

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   int result;                   /* Result to be returned */

/* Check the global error status. */
   if ( !astOK ) return 0;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's current Frame. */
   fr = astGetFrame( this->frameset, AST__CURRENT );

/* Obtain the number of axes in this Frame. */
   result = astGetNaxes( fr );

/* Annul the current Frame pointer. */
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = 0;

/* Return the result. */
   return result;
}

static const int *GetPerm( AstFrame *this_frame ) {
/*
*  Name:
*     GetPerm

*  Purpose:
*     Access the axis permutation array for the current Frame of a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     const int *GetPerm( AstFrame *this )

*  Class Membership:
*     Region member function (over-rides the astGetPerm protected
*     method inherited from the Frame class).

*  Description:
*     This function returns a pointer to the axis permutation array
*     for the current Frame of a Region. This array constitutes a
*     lookup-table that converts between an axis number supplied
*     externally and the corresponding index in the Frame's internal
*     axis arrays.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     Pointer to the current Frame's axis permutation array (a
*     constant array of int).  Each element of this contains the
*     (zero-based) internal axis index to be used in place of the
*     external index which is used to address the permutation
*     array. If the current Frame has zero axes, this pointer will be
*     NULL.

*  Notes:
*     - The pointer returned by this function gives direct access to
*     data internal to the Frame object. It remains valid only so long
*     as the Frame exists. The permutation array contents may be
*     modified by other functions which operate on the Frame and this
*     may render the returned pointer invalid.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to Region structure */
   const int *result;            /* Result pointer value */

/* Check the global error status. */
   if ( !astOK ) return NULL;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's current Frame and then obtain a
   pointer to its axis permutation array. Annul the Frame pointer
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astGetPerm( fr );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = NULL;

/* Return the result. */
   return result;
}

static AstFrame *GetRegionFrame( AstRegion *this ) {
/*
*++
*  Name:
c     astGetRegionFrame
f     AST_GETREGIONFRAME

*  Purpose:
*     Obtain a pointer to the encapsulated Frame within a Region.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "region.h"
c     AstFrame *astGetRegionFrame( AstRegion *this )
f     RESULT = AST_GETREGIONFRAME( THIS, STATUS )

*  Class Membership:
*     Region method.

*  Description:
*     This function returns a pointer to the Frame represented by a
*     Region.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Region.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Returned Value:
c     astGetRegionFrame()
f     AST_GETREGIONFRAME = INTEGER
*        A pointer to a deep copy of the Frame represented by the Region.
*        Using this pointer to modify the Frame will have no effect on
*        the Region. To modify the Region, use the Region pointer directly.

*  Notes:
*     - A null Object pointer (AST__NULL) will be returned if this
c     function is invoked with the AST error status set, or if it
f     function is invoked with STATUS set to an error value, or if it
*     should fail for any reason.
*--
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstFrame *result;             /* Pointer value to return */

/* Initialise. */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Get a pointer to the current Frame of the encapsulated FrameSet. */
   fr = astGetFrame( this->frameset, AST__CURRENT );

/* Take a deep copy of it, and then annul the original pointer. */
   result = astCopy( fr );
   fr = astAnnul( fr );

/* If not OK, annul the returned pointer. */
   if( !astOK ) result = astAnnul( result );

/* Return the result. */
   return result;
}

void astInitRegionVtab_(  AstRegionVtab *vtab, const char *name ) {
/*
*+
*  Name:
*     astInitRegionVtab

*  Purpose:
*     Initialise a virtual function table for a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     void astInitRegionVtab( AstRegionVtab *vtab, const char *name )

*  Class Membership:
*     Region vtab initialiser.

*  Description:
*     This function initialises the component of a virtual function
*     table which is used by the Region class.

*  Parameters:
*     vtab
*        Pointer to the virtual function table. The components used by
*        all ancestral classes will be initialised if they have not already
*        been initialised.
*     name
*        Pointer to a constant null-terminated character string which contains
*        the name of the class to which the virtual function table belongs (it 
*        is this pointer value that will subsequently be returned by the Object
*        astClass function).
*-
*/

/* Local Variables: */
   AstFrameVtab *frame;          /* Pointer to Frame component of Vtab */
   AstMappingVtab *mapping;      /* Pointer to Mapping component of Vtab */
   AstObjectVtab *object;        /* Pointer to Object component of Vtab */

/* Check the local error status. */
   if ( !astOK ) return;

/* Initialize the component of the virtual function table used by the
   parent class. */
   astInitFrameVtab( (AstFrameVtab *) vtab, name );

/* Store a unique "magic" value in the virtual function table. This
   will be used (by astIsARegion) to determine if an object belongs
   to this class.  We can conveniently use the address of the (static)
   class_init variable to generate this unique value. */
   vtab->check = &class_init;

/* Initialise member function pointers. */
/* ------------------------------------ */

/* Store pointers to the member functions (implemented here) that
   provide virtual methods for this class. */
   vtab->ClearNegated = ClearNegated;
   vtab->GetNegated = GetNegated;
   vtab->SetNegated = SetNegated;
   vtab->TestNegated = TestNegated;

   vtab->ClearRegionFS = ClearRegionFS;
   vtab->GetRegionFS = GetRegionFS;
   vtab->SetRegionFS = SetRegionFS;
   vtab->TestRegionFS = TestRegionFS;

   vtab->ClearClosed = ClearClosed;
   vtab->GetClosed = GetClosed;
   vtab->SetClosed = SetClosed;
   vtab->TestClosed = TestClosed;

   vtab->ClearMeshSize = ClearMeshSize;
   vtab->GetMeshSize = GetMeshSize;
   vtab->SetMeshSize = SetMeshSize;
   vtab->TestMeshSize = TestMeshSize;

   vtab->ClearFillFactor = ClearFillFactor;
   vtab->GetFillFactor = GetFillFactor;
   vtab->SetFillFactor = SetFillFactor;
   vtab->TestFillFactor = TestFillFactor;

   vtab->DumpUnc = DumpUnc;
   vtab->GetBounded = GetBounded;
   vtab->TestUnc = TestUnc;
   vtab->ClearUnc = ClearUnc;
   vtab->GetRegionFrame = GetRegionFrame;
   vtab->MapRegion = MapRegion;
   vtab->Overlap = Overlap;
   vtab->OverlapX = OverlapX;
   vtab->Negate = Negate;
   vtab->BndBaseMesh = BndBaseMesh;
   vtab->RegBaseMesh = RegBaseMesh;
   vtab->RegBaseBox = RegBaseBox;
   vtab->RegCentre = RegCentre;
   vtab->RegMesh = RegMesh;
   vtab->GetDefUnc = GetDefUnc;
   vtab->GetUnc = GetUnc;
   vtab->SetUnc = SetUnc;
   vtab->RegCurBox = RegCurBox;
   vtab->RegOverlay = RegOverlay;
   vtab->RegFrame = RegFrame;
   vtab->RegPins = RegPins;
   vtab->RegTransform = RegTransform;
   vtab->SetRegFS = SetRegFS;
   vtab->MaskB = MaskB;
   vtab->MaskD = MaskD;
   vtab->MaskF = MaskF;
   vtab->MaskI = MaskI;
   vtab->MaskL = MaskL;
   vtab->MaskS = MaskS;
   vtab->MaskUB = MaskUB;
   vtab->MaskUI = MaskUI;
   vtab->MaskUL = MaskUL;
   vtab->MaskUS = MaskUS;
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
   vtab->MaskLD = MaskLD;
#endif

/* Save the inherited pointers to methods that will be extended, and store 
   replacement pointers for methods which will be over-ridden by new member 
   functions implemented here. */
   object = (AstObjectVtab *) vtab;
   mapping = (AstMappingVtab *) vtab;
   frame = (AstFrameVtab *) vtab;

   parent_getnaxes = frame->GetNaxes;

   object->Equal = Equal;
   object->ClearAttrib = ClearAttrib;
   object->GetAttrib = GetAttrib;
   object->SetAttrib = SetAttrib;
   object->TestAttrib = TestAttrib;

   mapping->ReportPoints = ReportPoints;
   mapping->Simplify = Simplify;

   frame->Abbrev = Abbrev;
   frame->Angle = Angle;
   frame->AxAngle = AxAngle;
   frame->AxDistance = AxDistance;
   frame->AxOffset = AxOffset;
   frame->CheckPerm = CheckPerm;
   frame->ClearDigits = ClearDigits;
   frame->ClearDirection = ClearDirection;
   frame->ClearDomain = ClearDomain;
   frame->ClearFormat = ClearFormat;
   frame->ClearLabel = ClearLabel;
   frame->ClearMatchEnd = ClearMatchEnd;
   frame->ClearMaxAxes = ClearMaxAxes;
   frame->ClearMinAxes = ClearMinAxes;
   frame->ClearPermute = ClearPermute;
   frame->ClearPreserveAxes = ClearPreserveAxes;
   frame->ClearSymbol = ClearSymbol;
   frame->ClearTitle = ClearTitle;
   frame->ClearUnit = ClearUnit;
   frame->Convert = Convert;
   frame->ConvertX = ConvertX;
   frame->Distance = Distance;
   frame->FindFrame = FindFrame;
   frame->Format = Format;
   frame->Gap = Gap;
   frame->GetAxis = GetAxis;
   frame->GetDigits = GetDigits;
   frame->GetDirection = GetDirection;
   frame->GetDomain = GetDomain;
   frame->GetFormat = GetFormat;
   frame->GetLabel = GetLabel;
   frame->GetMatchEnd = GetMatchEnd;
   frame->GetMaxAxes = GetMaxAxes;
   frame->GetMinAxes = GetMinAxes;
   frame->GetNaxes = GetNaxes;
   frame->GetPerm = GetPerm;
   frame->GetPermute = GetPermute;
   frame->GetPreserveAxes = GetPreserveAxes;
   frame->GetSymbol = GetSymbol;
   frame->GetTitle = GetTitle;
   frame->GetUnit = GetUnit;
   frame->IsUnitFrame = IsUnitFrame;
   frame->Match = Match;
   frame->Norm = Norm;
   frame->Offset = Offset;
   frame->Offset2 = Offset2;
   frame->Overlay = Overlay;
   frame->PermAxes = PermAxes;
   frame->PickAxes = PickAxes;
   frame->Resolve = Resolve;
   frame->ResolvePoints = ResolvePoints;
   frame->SetAxis = SetAxis;
   frame->SetDigits = SetDigits;
   frame->SetDirection = SetDirection;
   frame->SetDomain = SetDomain;
   frame->SetFormat = SetFormat;
   frame->SetLabel = SetLabel;
   frame->SetMatchEnd = SetMatchEnd;
   frame->SetMaxAxes = SetMaxAxes;
   frame->SetMinAxes = SetMinAxes;
   frame->SetPermute = SetPermute;
   frame->SetPreserveAxes = SetPreserveAxes;
   frame->SetSymbol = SetSymbol;
   frame->SetTitle = SetTitle;
   frame->SetUnit = SetUnit;
   frame->SubFrame = SubFrame;
   frame->SystemCode = SystemCode;
   frame->SystemString = SystemString;
   frame->TestDigits = TestDigits;
   frame->TestDirection = TestDirection;
   frame->TestDomain = TestDomain;
   frame->TestFormat = TestFormat;
   frame->TestLabel = TestLabel;
   frame->TestMatchEnd = TestMatchEnd;
   frame->TestMaxAxes = TestMaxAxes;
   frame->TestMinAxes = TestMinAxes;
   frame->TestPermute = TestPermute;
   frame->TestPreserveAxes = TestPreserveAxes;
   frame->TestSymbol = TestSymbol;
   frame->TestTitle = TestTitle;
   frame->TestUnit = TestUnit;
   frame->Unformat = Unformat;
   frame->ValidateAxis = ValidateAxis;
   frame->ValidateAxisSelection = ValidateAxisSelection;
   frame->ValidateSystem = ValidateSystem;

   frame->GetActiveUnit = GetActiveUnit;
   frame->SetActiveUnit = SetActiveUnit;
   frame->TestActiveUnit = TestActiveUnit;

   frame->GetTop = GetTop;
   frame->SetTop = SetTop;
   frame->TestTop = TestTop;
   frame->ClearTop = ClearTop;

   frame->GetBottom = GetBottom;
   frame->SetBottom = SetBottom;
   frame->TestBottom = TestBottom;
   frame->ClearBottom = ClearBottom;

   frame->GetEpoch = GetEpoch;
   frame->SetEpoch = SetEpoch;
   frame->TestEpoch = TestEpoch;
   frame->ClearEpoch = ClearEpoch;

   frame->GetSystem = GetSystem;
   frame->SetSystem = SetSystem;
   frame->TestSystem = TestSystem;
   frame->ClearSystem = ClearSystem;

   frame->GetAlignSystem = GetAlignSystem;
   frame->SetAlignSystem = SetAlignSystem;
   frame->TestAlignSystem = TestAlignSystem;
   frame->ClearAlignSystem = ClearAlignSystem;

/* Declare the copy constructor, destructor and class dump
   functions. */
   astSetDelete( vtab, Delete );
   astSetCopy( vtab, Copy );
   astSetDump( vtab, Dump, "Region",
               "An area within a coordinate system" );
}

static int IsUnitFrame( AstFrame *this ){
/*
*  Name:
*     IsUnitFrame

*  Purpose:
*     Is this Frame equivalent to a UnitMap?

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int IsUnitFrame( AstFrame *this )

*  Class Membership:
*     Region member function (over-rides the protected astIsUnitFrame
*     method inherited from the Frame class).

*  Description:
*     This function returns a flag indicating if the supplied Frame is
*     equivalent to a UnitMap when treated as a Mapping (note, the Frame
*     class inherits from Mapping and therefore every Frame is also a Mapping).

*  Parameters:
*     this 
*        Pointer to the Frame.

*  Returned Value:
*     A non-zero value is returned if the supplied Frame is equivalent to
*     a UnitMap when treated as a Mapping.

*-
*/

/* Check the global error status. */
   if( !astOK ) return 0;

/* The Region class is never equivalent to a UnitMap. */
   return 0;
}

static AstRegion *MapRegion( AstRegion *this, AstMapping *map,
                             AstFrame *frame ) {
/*
*+
*  Name:
*     astMapRegion

*  Purpose:
*     Transform a Region into a new Frame using a given Mapping.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "region.h"
*     AstRegion *astMapRegion( AstRegion *this, AstMapping *map,
*                              AstFrame *frame ) 

*  Class Membership:
*     Region method.

*  Description:
*     This function returns a pointer to a new Region which corresponds to
*     supplied Region in some other specified coordinate system. A
*     Mapping is supplied which transforms positions between the old and new
*     coordinate systems. The new Region may not be of the same class as
*     the original region.

*  Parameters:
*     this
*        Pointer to the Region.
*     map
*        Pointer to a Mapping which transforms positions from the
*        coordinate system represented by the supplied Region to the 
*        coordinate system specified by "frame". The supplied Mapping should 
*        define both forward and inverse transformations, and these 
*        transformations should form a genuine inverse pair. That is, 
*        transforming a position using the forward transformation and then 
*        using the inverse transformation should produce the original input 
*        position. Some Mapping classes (such as PermMap, MathMap, SphMap) 
*        can result in Mappings for which this is not true.
*     frame
*        Pointer to a Frame describing the coordinate system in which 
*        the new Region is required.

*  Returned Value:
*     astMapRegion()
*        A pointer to a new Region. This Region will represent the area
*        within the coordinate system specified by "frame" which corresponds 
*        to the supplied Region.

*  Notes:
*     - This is the protected implementation of this function - it does
*     not simplify the returned Region. The public implementation is 
*     astMapRegionID, which simplifies the returned Region.
*     - A null Object pointer (AST__NULL) will be returned if this
*     function is invoked with the AST error status set, or if it
*     should fail for any reason.
*-
*/

/* Local Variables: */
   AstFrameSet *fs;               /* Pointer to FrameSet */
   AstRegion *result;             /* Pointer value to return */
   int icurr;                     /* Index of current Frame */

/* Initialise. */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* First check the Mapping is suitable. It must defined both a forward
   and an inverse Mapping. */
   if( !astGetTranInverse( map ) ) {
      astError( AST__NODEF, "astMapRegion(%s): The supplied %s does not "
                "define an inverse transformation.", astGetClass( this ),
                astGetClass( map ) );
   } else if( !astGetTranForward( map ) ) {
      astError( AST__NODEF, "astMapRegion(%s): The supplied %s does not "
                "define a forward transformation.", astGetClass( this ),
                astGetClass( map ) );
   }

/* Take a deep copy of the supplied Region. */
   result = astCopy( this );

/* Get a pointer to the encapsulated FrameSet. */
   if( astOK ) {
      fs = result->frameset;

/* Add in the new Frame and Mapping. First note the index of the original
   current Frame. */
      icurr = astGetCurrent( fs );
      astAddFrame( fs, AST__CURRENT, map, frame );

/* Remove the original current Frame. */
      astRemoveFrame( fs, icurr );

/* The base and current Frames of the resulting FrameSet are now (in
   general) different and so the Region should include its FrameSet in any
   Dump. */
      astSetRegionFS( result, 1 );
   }

/* If not OK, annul the returned pointer. */
   if( !astOK ) result = astAnnul( result );

/* Return the result. */
   return result;
}

/*
*++
*  Name:
c     astMask<X>
f     AST_MASK<X>

*  Purpose:
*     Mask a region of a data grid.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "region.h"
c     int astMask<X>( AstRegion *this, AstMapping *map, int inside, int ndim, 
c                     const int lbnd[], const int ubnd[], <Xtype> in[], 
c                     <Xtype> val )
f     RESULT = AST_MASK<X>( THIS, MAP, INSIDE, NDIM, LBND, UBND, IN, VAL, 
f                           STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
*     This is a set of functions for masking out regions within gridded data 
*     (e.g. an image). The functions modifies a given data grid by
*     assigning a specified value to all samples which are inside (or outside 
c     if "inside" is zero) 
f     if INSIDE is .FALSE.) 
*     the specified Region.
*
*     You should use a masking function which matches the numerical
*     type of the data you are processing by replacing <X> in
c     the generic function name astMask<X> by an appropriate 1- or
f     the generic function name AST_MASK<X> by an appropriate 1- or
*     2-character type code. For example, if you are masking data
c     with type "float", you should use the function astMaskF (see
f     with type REAL, you should use the function AST_MASKR (see
*     the "Data Type Codes" section below for the codes appropriate to
*     other numerical types).

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to a Region. 
c     map
f     MAP = INTEGER (Given)
*        Pointer to a Mapping. The forward transformation should map
*        positions in the coordinate system of the supplied Region
*        into pixel coordinates as defined by the 
c        "lbnd" and "ubnd" parameters. A NULL pointer
f        LBND and UBND arguments. A value of AST__NULL 
*        can be supplied if the coordinate system of the supplied Region 
*        corresponds to pixel coordinates. This is equivalent to
*        supplying a UnitMap.
*
*        The number of inputs for this Mapping (as given by its Nin attribute) 
*        should match the number of axes in the supplied Region (as given
*        by the Naxes attribute of the Region).
*        The number of outputs for the Mapping (as given by its Nout attribute) 
*        should match the number of
c        grid dimensions given by the value of "ndim"
f        grid dimensions given by the value of NDIM 
*        below. 
c     inside
f     INSIDE = INTEGER (Given)
*        A boolean value which indicates which pixel are to be masked. If 
c        a non-zero value 
f        .TRUE.
*        is supplied, then all grid pixels with centres inside the supplied 
*        Region are assigned the value given by
c        "val",
f        VAL,
*        and all other pixels are left unchanged. If 
c        zero 
f        .FALSE.
*        is supplied, then all grid pixels with centres not inside the supplied 
*        Region are assigned the value given by
c        "val",
f        VAL,
*        and all other pixels are left unchanged. Note, the Negated
*        attribute of the Region is used to determine which pixel are
*        inside the Region and which are outside. So the inside of a Region 
*        which has not been negated is the same as the outside of the 
*        corresponding negated Region.
*
*        For types of Region such as PointList which have zero volume,
*        pixel centres will rarely fall exactly within the Region. For
*        this reason, the inclusion criterion is changed for zero-volume
*        Regions so that pixels are included (or excluded) if any part of
*        the Region passes through the pixel. For a PointList, this means
*        that pixels are included (or excluded) if they contain at least
*        one of the points listed in the PointList.
c     ndim
f     NDIM = INTEGER (Given)
*        The number of dimensions in the input grid. This should be at
*        least one.
c     lbnd
f     LBND( NDIM ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim" elements,
f        An array
*        containing the coordinates of the centre of the first pixel
*        in the input grid along each dimension.
c     ubnd
f     UBND( NDIM ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim" elements,
f        An array
*        containing the coordinates of the centre of the last pixel in
*        the input grid along each dimension.
*
c        Note that "lbnd" and "ubnd" together define the shape
f        Note that LBND and UBND together define the shape
*        and size of the input grid, its extent along a particular
c        (j'th) dimension being ubnd[j]-lbnd[j]+1 (assuming the
c        index "j" to be zero-based). They also define
f        (J'th) dimension being UBND(J)-LBND(J)+1. They also define
*        the input grid's coordinate system, each pixel having unit
*        extent along each dimension with integral coordinate values
*        at its centre.
c     in
f     IN( * ) = <Xtype> (Given and Returned)
c        Pointer to an array, with one element for each pixel in the
f        An array, with one element for each pixel in the
*        input grid, containing the data to be masked.  The
*        numerical type of this array should match the 1- or
*        2-character type code appended to the function name (e.g. if
c        you are using astMaskF, the type of each array element
c        should be "float").
f        you are using AST_MASKR, the type of each array element
f        should be REAL).
*
*        The storage order of data within this array should be such
*        that the index of the first grid dimension varies most
*        rapidly and that of the final dimension least rapidly
c        (i.e. Fortran array indexing is used).
f        (i.e. normal Fortran array storage order).
*
*        On exit, the samples specified by
c        "inside" are set to the value of "val".
f        INSIDE are set to the value of VAL. 
*        All other samples are left unchanged.
c     val
f     VAL = <Xtype> (Given)
*        This argument should have the same type as the elements of
c        the "in" array. It specifies the value used to flag the
f        the IN array. It specifies the value used to flag the
*        masked data (see
c        "inside").
f        INSIDE).
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Returned Value:
c     astMask<X>()
f     AST_MASK<X> = INTEGER
*        The number of pixels to which a value of 
c        "badval" 
f        BADVAL 
*        has been assigned.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.

*  Data Type Codes:
*     To select the appropriate masking function, you should
c     replace <X> in the generic function name astMask<X> with a
f     replace <X> in the generic function name AST_MASK<X> with a
*     1- or 2-character data type code, so as to match the numerical
*     type <Xtype> of the data you are processing, as follows:
c     - D: double
c     - F: float
c     - L: long int
c     - UL: unsigned long int
c     - I: int
c     - UI: unsigned int
c     - S: short int
c     - US: unsigned short int
c     - B: byte (signed char)
c     - UB: unsigned byte (unsigned char)
f     - D: DOUBLE PRECISION
f     - R: REAL
f     - I: INTEGER
f     - UI: INTEGER (treated as unsigned)
f     - S: INTEGER*2 (short integer)
f     - US: INTEGER*2 (short integer, treated as unsigned)
f     - B: BYTE (treated as signed)
f     - UB: BYTE (treated as unsigned)
*
c     For example, astMaskD would be used to process "double"
c     data, while astMaskS would be used to process "short int"
c     data, etc.
f     For example, AST_MASKD would be used to process DOUBLE
f     PRECISION data, while AST_MASKS would be used to process
f     short integer data (stored in an INTEGER*2 array), etc.
f
f     For compatibility with other Starlink facilities, the codes W
f     and UW are provided as synonyms for S and US respectively (but
f     only in the Fortran interface to AST).

*--
*/
/* Define a macro to implement the function for a specific data
   type. */
#define MAKE_MASK(X,Xtype) \
static int Mask##X( AstRegion *this, AstMapping *map, int inside, int ndim, \
                    const int lbnd[], const int ubnd[], \
                    Xtype in[], Xtype val ) { \
\
/* Local Variables: */ \
   AstFrame *grid_frame;         /* Pointer to Frame describing grid coords */ \
   AstRegion *used_region;       /* Pointer to Region to be used by astResample */ \
   Xtype *c;                     /* Pointer to next array element */ \
   Xtype *d;                     /* Pointer to next array element */ \
   Xtype *out;                   /* Pointer to the array used for resample output */ \
   Xtype *tmp_out;               /* Pointer to temporary output array */ \
   double *lbndgd;               /* Pointer to array holding lower grid bounds */ \
   double *ubndgd;               /* Pointer to array holding upper grid bounds */ \
   int *lbndg;                   /* Pointer to array holding lower grid bounds */ \
   int *ubndg;                   /* Pointer to array holding upper grid bounds */ \
   int idim;                     /* Loop counter for coordinate dimensions */ \
   int ipix;                     /* Loop counter for pixel index */ \
   int nax;                      /* Number of Region axes */ \
   int nin;                      /* Number of Mapping input coordinates */ \
   int nout;                     /* Number of Mapping output coordinates */ \
   int npix;                     /* Number of pixels in supplied array */ \
   int npixg;                    /* Number of pixels in bounding box */ \
   int result;                   /* Result value to return */ \
\
/* Initialise. */ \
   result = 0; \
\
/* Check the global error status. */ \
   if ( !astOK ) return result; \
\
/* Obtain value for the Naxes attribute of the Region. */ \
   nax = astGetNaxes( this ); \
\
/* If supplied, obtain values for the Nin and Nout attributes of the Mapping. */ \
   if( map ) { \
      nin = astGetNin( map ); \
      nout = astGetNout( map ); \
\
/* If OK, check that the number of mapping inputs matches the \
   number of axes in the Region. Report an error if necessary. */ \
      if ( astOK && ( nax != nin ) ) { \
         astError( AST__NGDIN, "astMask"#X"(%s): Bad number of mapping " \
                   "inputs (%d).", astGetClass( this ), nin ); \
         astError( AST__NGDIN, "The %s given requires %d coordinate value%s " \
                   "to specify a position.", \
                   astGetClass( this ), nax, ( nax == 1 ) ? "" : "s" ); \
      } \
\
/* If OK, check that the number of mapping outputs matches the \
   number of grid dimensions. Report an error if necessary. */ \
      if ( astOK && ( ndim != nout ) ) { \
         astError( AST__NGDIN, "astMask"#X"(%s): Bad number of mapping " \
                   "outputs (%d).", astGetClass( this ), nout ); \
         astError( AST__NGDIN, "The pixel grid requires %d coordinate value%s " \
                   "to specify a position.", \
                   ndim, ( ndim == 1 ) ? "" : "s" ); \
      } \
\
/* Create a new Region by mapping the supplied Region with the supplied \
   Mapping. The resulting Region represents a region in grid coordinates. */ \
      grid_frame = astFrame( ndim, "Domain=grid" ); \
      used_region = astMapRegion( this, map, grid_frame ); \
      grid_frame = astAnnul( grid_frame ); \
\
/* If no Mapping was supplied check that the number of grid dimensions \
   matches the number of axes in the Region.*/ \
   } else if ( astOK && ( ( ndim != nax ) || ( ndim < 1 ) ) ) { \
      used_region = NULL; \
      astError( AST__NGDIN, "astMask"#X"(%s): Bad number of input grid " \
                "dimensions (%d).", astGetClass( this ), ndim ); \
      if ( ndim != nax ) { \
         astError( AST__NGDIN, "The %s given requires %d coordinate value%s " \
                   "to specify an input position.", \
                   astGetClass( this ), nax, ( nax == 1 ) ? "" : "s" ); \
      } \
\
/* If no Mapping was supplied and the parameters look OK, clone the \
   supplied Region pointer for use later on. */ \
   } else { \
      used_region = astClone( this ); \
   } \
\
/* Check that the lower and upper bounds of the input grid are \
   consistent. Report an error if any pair is not. */ \
   if ( astOK ) { \
      for ( idim = 0; idim < ndim; idim++ ) { \
         if ( lbnd[ idim ] > ubnd[ idim ] ) { \
            astError( AST__GBDIN, "astMask"#X"(%s): Lower bound of " \
                      "input grid (%d) exceeds corresponding upper bound " \
                      "(%d).", astGetClass( this ), \
                      lbnd[ idim ], ubnd[ idim ] ); \
            astError( AST__GBDIN, "Error in input dimension %d.", \
                      idim + 1 ); \
            break; \
         } \
      } \
   } \
\
/* Allocate memory, and then get the bounding box of this new Region in its \
   current Frame (grid coordinates). This bounding box assumes the region \
   has not been negated. */ \
   lbndg = astMalloc( sizeof( int )*(size_t) ndim ); \
   ubndg = astMalloc( sizeof( int )*(size_t) ndim ); \
   lbndgd = astMalloc( sizeof( double )*(size_t) ndim ); \
   ubndgd = astMalloc( sizeof( double )*(size_t) ndim ); \
   if( astOK ) { \
      astRegCurBox( used_region, lbndgd, ubndgd ); \
\
/* We convert the floating point bounds to integer pixel bounds, and at \
   the same time expand the box by 2 pixels at each edge to ensure that \
   rounding errors etc do not cause any of the Region to fall outside (or \
   on) the box. Do not let the expanded box extend outside the supplied \
   array bounds. Also note the total number of pixels in the supplied \
   array, and in the bounding box. */ \
      npix = 1; \
      npixg = 1; \
      for ( idim = 0; idim < ndim; idim++ ) { \
         lbndg[ idim ] = MAX( lbnd[ idim ], (int)( lbndgd[ idim ] + 0.5 ) - 2 ); \
         ubndg[ idim ] = MIN( ubnd[ idim ], (int)( ubndgd[ idim ] + 0.5 ) + 2 ); \
         npix *= ( ubnd[ idim ] - lbnd[ idim ] + 1 ); \
         npixg *= ( ubndg[ idim ] - lbndg[ idim ] + 1 ); \
      } \
\
/* All points outside this box are either all inside, or all outside, the \
   Region. So we can speed up processing by setting all the points which are \
   outside the box to the supplied data value (if required). This is \
   faster than checking each point individually using the Transform method \
   of the Region. We do this by supplying an alternative output array to \
   the resampling function below, which has been pre-filled with "val" at \
   every pixel. */ \
      if( ( inside != 0 ) == ( astGetNegated( used_region ) != 0 ) ) { \
\
/* Allocate memory for the alternative output array, and fill it with \
   "val". */ \
         tmp_out = astMalloc( sizeof( Xtype )*(size_t) npix ); \
         if( tmp_out ) { \
            c = tmp_out; \
            for( ipix = 0; ipix < npix; ipix++ ) *(c++) = val; \
            result = npix - npixg; \
         } \
\
/* Indicate that we will use this temporary array rather than the \
   supplied array. */ \
         out = tmp_out; \
\
/* If the outside of the grid box is outside the region of interest it \
   will be unchanged in the resturned array. Therefore we can use the \
   supplied array as the output array below. */ \
      } else { \
         tmp_out = NULL; \
         out = in; \
      } \
\
/* Temporarily invert the Region if required. The Region Transform methods \
   leave interior points unchanged and assign AST__BAD to exterior points. \
   This is the opposite of what we want (which is to leave exterior \
   points unchanged and assign VAL to interior points), so we negate the \
   region if the inside is to be assigned the value VAL.*/ \
      if( inside ) astNegate( used_region ); \
\
/* Invoke astResample to mask just the region inside the bounding box found \
   above (specified by lbndg and ubndg), since all the points outside this \
   box will already contain their required value. */ \
      result += astResample##X( used_region, ndim, lbnd, ubnd, in, in, AST__NEAREST, \
                                NULL, NULL, 0, 0.0, 100, val, ndim, \
                                lbnd, ubnd, lbndg, ubndg, out, out ); \
\
/* Revert to the original setting of the Negated attribute. */ \
      if( inside ) astNegate( used_region ); \
\
/* If required, copy the output data from the temporary output array to \
   the supplied array, and then free the temporary output array. */ \
      if( tmp_out ) { \
         c = tmp_out; \
         d = in; \
         for( ipix = 0; ipix < npix; ipix++ ) *(d++) = *(c++); \
         tmp_out = astFree( tmp_out ); \
      }\
   }\
\
/* Free resources */ \
   ubndg = astFree( ubndg ); \
   lbndg = astFree( lbndg ); \
   ubndgd = astFree( ubndgd ); \
   lbndgd = astFree( lbndgd ); \
   used_region = astAnnul( used_region ); \
\
/* If an error occurred, clear the returned result. */ \
   if ( !astOK ) result = 0; \
\
/* Return the result. */ \
   return result; \
}

/* Expand the above macro to generate a function for each required
   data type. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_MASK(LD,long double)
#endif
MAKE_MASK(D,double)
MAKE_MASK(F,float)
MAKE_MASK(L,long int)
MAKE_MASK(UL,unsigned long int)
MAKE_MASK(I,int)
MAKE_MASK(UI,unsigned int)
MAKE_MASK(S,short int)
MAKE_MASK(US,unsigned short int)
MAKE_MASK(B,signed char)
MAKE_MASK(UB,unsigned char)

/* Undefine the macro. */
#undef MAKE_MASK

static int Match( AstFrame *this_frame, AstFrame *target,
                  int **template_axes, int **target_axes,
                  AstMapping **map, AstFrame **result ) {
/*
*  Name:
*     Match

*  Purpose:
*     Determine if conversion is possible between two coordinate systems.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int Match( AstFrame *template, AstFrame *target,
*                int **template_axes, int **target_axes,
*                AstMapping **map, AstFrame **result )

*  Class Membership:
*     Region member function (over-rides the protected astMatch
*     method inherited from the Frame class).

*  Description:
*     This function matches the current Frame of a "template" Region
*     to a "target" frame and determines whether it is possible to
*     convert coordinates between them.  If it is, a Mapping that
*     performs the transformation is returned along with a new Frame
*     that describes the coordinate system that results when this
*     Mapping is applied to the current Frame of the target
*     Region. In addition, information is returned to allow the axes
*     in this "result" Frame to be associated with the corresponding
*     axes in the target and template Frames from which they are
*     derived.

*  Parameters:
*     template
*        Pointer to the template Region, whose current Frame
*        describes the coordinate system (or set of possible
*        coordinate systems) into which we wish to convert our
*        coordinates.
*     target
*        Pointer to the target Frame. This describes the coordinate
*        system in which we already have coordinates.
*     template_axes
*        Address of a location where a pointer to int will be returned
*        if the requested coordinate conversion is possible. This
*        pointer will point at a dynamically allocated array of
*        integers with one element for each axis of the "result" Frame
*        (see below). It must be freed by the caller (using astFree)
*        when no longer required.
*
*        For each axis in the result Frame, the corresponding element
*        of this array will return the index of the axis in the
*        template Region's current Frame from which it is
*        derived. If it is not derived from any template Region
*        axis, a value of -1 will be returned instead.
*     target_axes
*        Address of a location where a pointer to int will be returned
*        if the requested coordinate conversion is possible. This
*        pointer will point at a dynamically allocated array of
*        integers with one element for each axis of the "result" Frame
*        (see below). It must be freed by the caller (using astFree)
*        when no longer required.
*
*        For each axis in the result Frame, the corresponding element
*        of this array will return the index of the target Frame axis
*        from which it is derived. If it is not derived from any
*        target Frame axis, a value of -1 will be returned instead.
*     map
*        Address of a location where a pointer to a new Mapping will
*        be returned if the requested coordinate conversion is
*        possible. If returned, the forward transformation of this
*        Mapping may be used to convert coordinates between the target
*        Frame and the result Frame (see below) and the inverse
*        transformation will convert in the opposite direction.
*     result
*        Address of a location where a pointer to a new Frame will be
*        returned if the requested coordinate conversion is
*        possible. If returned, this Frame describes the coordinate
*        system that results from applying the returned Mapping
*        (above) to the "target" coordinate system. In general, this
*        Frame will combine attributes from (and will therefore be
*        more specific than) both the target Frame and the current
*        Frame of the template Region. In particular, when the
*        template allows the possibility of transformaing to any one
*        of a set of alternative coordinate systems, the "result"
*        Frame will indicate which of the alternatives was used.

*  Returned Value:
*     A non-zero value is returned if the requested coordinate
*     conversion is possible. Otherwise zero is returned (this will
*     not in itself result in an error condition).

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to Region's current Frame */
   int match;                    /* Result to be returned */

/* Initialise the returned values. */
   *template_axes = NULL;
   *target_axes = NULL;
   *map = NULL;
   *result = NULL;
   match = 0;

/* Check the global error status. */
   if ( !astOK ) return match;

/* Invoke the parent astMatch method on the current Frame within the
   encapsulated FrameSet within the Region. */
   fr = astGetFrame( ((AstRegion *) this_frame)->frameset, AST__CURRENT );
   match = Match( fr, target, template_axes, target_axes, map, result );
   fr = astAnnul( fr );

/* Return the result. */
   return match;
}

static void Negate( AstRegion *this ) {
/*
*++
*  Name:
c     astNegate
f     AST_NEGATE

*  Purpose:
*     Negate the area represented by a Region.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "region.h"
c     void astNegate( AstRegion *this )
f     CALL AST_NEGATE( THIS, STATUS )

*  Class Membership:
*     Region method.

*  Description:
*     This function negates the area represented by a Region. That is,
*     points which were previously inside the region will then be
*     outside, and points which were outside will be inside. This is
*     acomplished by toggling the state of the Negated attribute for
*     the supplied region.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Region.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*--
*/

/* Check the global error status. */
   if ( !astOK ) return;

/* Toggle the Negated attribute. */
   astSetNegated( this, astGetNegated( this ) ? 0 : 1 );

}

static void Norm( AstFrame *this_frame, double value[] ) {
/*
*  Name:
*     Norm

*  Purpose:
*     Normalise a set of Region coordinates.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void Norm( AstAxis *this, double value[] )

*  Class Membership:
*     Region member function (over-rides the astNorm method
*     inherited from the Frame class).

*  Description:
*     This function converts a set of coordinate values for the
*     current Frame of a Region, which might potentially be
*     unsuitable for display to a user (for instance, may lie outside
*     the expected range of values) into a set of acceptable
*     alternative values suitable for display.
*
*     Typically, for Frames whose axes represent cyclic values (such
*     as angles or positions on the sky), this function wraps an
*     arbitrary set of coordinates, so that they lie within the first
*     cycle (say zero to 2*pi or -pi/2 to +pi/2). For Frames with
*     ordinary linear axes, without constraints, this function will
*     typically return the original coordinate values unchanged.

*  Parameters:
*     this
*        Pointer to the Region.
*     value
*        An array of double, with one element for each Region axis.
*        This should contain the initial set of coordinate values,
*        which will be modified in place.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to the current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's current Frame and invoke this
   Frame's astNorm method to obtain the new values. Annul the Frame
   pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astNorm( fr, value );
   fr = astAnnul( fr );
}

static void Offset( AstFrame *this_frame, const double point1[],
                    const double point2[], double offset, double point3[] ) {
/*
*  Name:
*     Offset

*  Purpose:
*     Calculate an offset along a geodesic curve.

*  Type:
*     Public virtual function.

*  Synopsis:
*     #include "region.h"
*     void Offset( AstFrame *this,
*                  const double point1[], const double point2[],
*                  double offset, double point3[] )

*  Class Membership:
*     Region member function (over-rides the protected astOffset
*     method inherited from the Frame class).

*  Description:
*     This function finds the Region coordinate values of a point
*     which is offset a specified distance along the geodesic curve
*     between two other points.

*  Parameters:
*     this
*        Pointer to the Region.
*     point1
*        An array of double, with one element for each Region axis.
*        This should contain the coordinates of the point marking the
*        start of the geodesic curve.
*     point2
*        An array of double, with one element for each Region axis
*        This should contain the coordinates of the point marking the
*        end of the geodesic curve.
*     offset
*        The required offset from the first point along the geodesic
*        curve. If this is positive, it will be towards the second
*        point. If it is negative, it will be in the opposite
*        direction. This offset need not imply a position lying
*        between the two points given, as the curve will be
*        extrapolated if necessary.
*     point3
*        An array of double, with one element for each Region axis
*        in which the coordinates of the required point will be
*        returned.

*  Notes:
*     - The geodesic curve used by this function is the path of
*     shortest distance between two points, as defined by the
*     astDistance function.
*     - This function will return "bad" coordinate values (AST__BAD)
*     if any of the input coordinates has this value.
*     - "Bad" coordinate values will also be returned if the two
*     points supplied are coincident (or otherwise fail to uniquely
*     specify a geodesic curve) but the requested offset is non-zero.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's current Frame and invoke this
   Frame's astOffset method. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astOffset( fr, point1, point2, offset, point3 );
   fr = astAnnul( fr );
}

static double Offset2( AstFrame *this_frame, const double point1[2], 
                       double angle, double offset, double point2[2] ){
/*
*  Name:
*     Offset2

*  Purpose:
*     Calculate an offset along a geodesic curve in a 2D Frame.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     double Offset2( AstFrame *this, const double point1[2], double angle,
*                     double offset, double point2[2] );

*  Class Membership:
*     Region member function (over-rides the protected astOffset2
*     method inherited from the Frame class).

*  Description:
*     This function finds the Frame coordinate values of a point which
*     is offset a specified distance along the geodesic curve at a
*     given angle from a specified starting point. It can only be
*     used with 2-dimensional Frames.
*
*     For example, in a basic Frame, this offset will be along the
*     straight line joining two points. For a more specialised Frame
*     describing a sky coordinate system, however, it would be along
*     the great circle passing through two sky positions.

*  Parameters:
*     this
*        Pointer to the Frame.
*     point1
*        An array of double, with one element for each Frame axis
*        (Naxes attribute). This should contain the coordinates of the
*        point marking the start of the geodesic curve.
*     angle
*        The angle (in radians) from the positive direction of the second
*        axis, to the direction of the required position, as seen from
*        the starting position. Positive rotation is in the sense of
*        rotation from the positive direction of axis 2 to the positive
*        direction of axis 1.
*     offset
*        The required offset from the first point along the geodesic
*        curve. If this is positive, it will be in the direction of the
*        given angle. If it is negative, it will be in the opposite
*        direction.
*     point2
*        An array of double, with one element for each Frame axis
*        in which the coordinates of the required point will be returned.

*  Returned Value:
*     The direction of the geodesic curve at the end point. That is, the
*     angle (in radians) between the positive direction of the second
*     axis and the continuation of the geodesic curve at the requested
*     end point. Positive rotation is in the sense of rotation from
*     the positive direction of axis 2 to the positive direction of axis 1.

*  Notes:
*     - The geodesic curve used by this function is the path of
*     shortest distance between two points, as defined by the
*     astDistance function.
*     - An error will be reported if the Frame is not 2-dimensional.
*     - This function will return "bad" coordinate values (AST__BAD)
*     if any of the input coordinates has this value.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double result;                /* Value to return */

/* Check the global error status. */
   if ( !astOK ) return AST__BAD;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke the
   astOffset2 method for this Frame. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astOffset2( fr, point1, angle, offset, point2 );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = AST__BAD;

/* Return the result. */
   return result;
}

static int Overlap( AstRegion *this, AstRegion *that ){
/*
*++
*  Name:
c     astOverlap
f     AST_OVERLAP

*  Purpose:
*     Test if two regions overlap each other.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "region.h"
c     int astOverlap( AstRegion *this, AstRegion *that ) 
f     RESULT = AST_OVERLAP( THIS, THAT, STATUS )

*  Class Membership:
*     Region method.

*  Description:
*     This function returns an integer value indicating if the two
*     supplied Regions overlap. The two Regions are converted to a commnon
*     coordinate system before performing the check. If this conversion is 
*     not possible (for instance because the two Regions represent areas in
*     different domains), then the check cannot be performed and a zero value 
*     is returned to indicate this.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the first Region.
c     that
f     THAT = INTEGER (Given)
*        Pointer to the second Region.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Returned Value:
c     astOverlap()
f     AST_OVERLAP = INTEGER
*        A value indicating if there is any overlap between the two Regions.
*        Possible values are:
*
*        0 - The check could not be performed because the second Region
*            could not be mapped into the coordinate system of the first
*            Region.
*
*        1 - There is no overlap between the two Regions.
*
*        2 - The first Region is completely inside the second Region.
*
*        3 - The second Region is completely inside the first Region.
*
*        4 - There is partial overlap between the two Regions.
*
*        5 - The Regions are identical to within their uncertainties.
*
*        6 - The second Region is the exact negation of the first Region
*            to within their uncertainties.

*  Notes:
*     - The returned values 5 and 6 do not check the value of the Closed 
*     attribute in the two Regions. 
*     - A value of zero will be returned if this function is invoked with the 
*     AST error status set, or if it should fail for any reason.
*--

* Implementation Notes:
*    This function is simply a wrap-up for the protected astOverlapX
*    method which performs the required processing but swaps the order
*    of the two arguments. This is a trick to allow the astOverlap method 
*    to be over-ridden by derived classes on the basis of the class of either 
*    of the two arguments.
*/

/* Check the global error status. */
   if ( !astOK ) return 0;

/* Invoke the "astOverlapX" method with the two arguments swapped. */
   return astOverlapX( that, this );
}

static int OverlapX( AstRegion *that, AstRegion *this ){
/*
*+
*  Name:
*     astOverlapX

*  Purpose:
*     Test if two regions overlap each other.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "region.h"
*     int astOverlapX( AstRegion *that, AstRegion *this ) 

*  Class Membership:
*     Region method.

*  Description:
*     This function performs the processing for the public astOverlap
*     method and has exactly the same interface except that the order
*     of the two arguments is swapped. This is a trick to allow
*     the astOverlap method to be over-ridden by derived classes on
*     the basis of the class of either of its two arguments.
*
*     See the astOverlap method for details of the interface.
*-
*/

/* Local Variables: */
   AstFrame *frm_reg1;            /* Pointer to "reg1" Frame */
   AstFrameSet *fs;               /* FrameSet connecting Region Frames */
   AstMapping *cmap;              /* Mapping connecting Region Frames */
   AstMapping *map;               /* Mapping form "reg2" current to "reg1" base */
   AstMapping *map_reg1;          /* Pointer to current->base Mapping in "reg1" */
   AstPointSet *ps1;              /* Mesh covering second Region */
   AstPointSet *ps2;              /* Mesh covering second Region */
   AstPointSet *ps3;              /* Mesh covering first Region */
   AstPointSet *ps4;              /* Mesh covering first Region */
   AstPointSet *reg2_mesh;        /* Mesh covering second Region */
   AstPointSet *reg1_mesh;        /* Central point within first Region */
   AstRegion *reg1;               /* Region to use as the first Region */
   AstRegion *reg2;               /* Region to use as the second Region */
   AstRegion *unc1;               /* "unc" mapped into Frame of first Region */
   AstRegion *unc;                /* Uncertainty in second Region */
   double **ptr;                  /* Pointer to pointset data */
   double *p;                     /* Pointer to next axis value */
   int allbad;                    /* Were all axis values bad? */
   int allgood;                   /* Were all axis values good? */
   int bnd1;                      /* Does reg1 have a finite boundary */
   int bnd_that;                  /* Does "that" have a finite boundary */
   int bnd_this;                  /* Does "this" have a finite boundary */
   int iax;                       /* Axis index */
   int ip;                        /* Index of point */
   int nc;                        /* Number of axis values per point */
   int np;                        /* Number of points in mesh */
   int result;                    /* Value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Return 5 if the two Regions are equal using the private Equal method. 
   Do not use the protected interface since that may have been extended. */
   if( Equal( this, that ) ) {
      return 5;

/* Return 6 if the two Regions are equal using the Equal method after
   temporarily negating the first. */
   } else {
      astNegate( this );
      result = Equal( this, that );
      astNegate( this );
      if( result ) return 6;
   }

/* The rest of this function tests for overlap by representing one of the 
   Regions as a mesh of points along its boundary, and then checking to see
   if any of the points in this mesh fall inside or outside the other Region. 
   This can only be done if the Region has a boundary of finite length (e.g. 
   Circles, Boxes, etc). Other Regions (e.g. Intervals) do not have finite 
   boundaries and consequently report an error if an attempt is made to 
   represent them using a boundary mesh. We now therefore check to see if 
   either of the two Regions has a finite boundary length. This will be the 
   case if the region is bounded, or if it can be made bounded simply by 
   negating it. If a Region is unbounded regardless of the setting of its 
   Negated flag, then it does not have a finite boundary. */
   bnd_this = astGetBounded( this );
   if( !bnd_this ) {
      astNegate( this );
      bnd_this = astGetBounded( this );
      astNegate( this );
   }

   bnd_that = astGetBounded( that );
   if( !bnd_that ) {
      astNegate( that );
      bnd_that = astGetBounded( that );
      astNegate( that );
   }

/* If neither Regions has a finite boundary, then we cannot currently
   determine any overlap, so report an error. Given more time, it is
   probably possible to think of some way of determining overlap between
   two unbounded Regions, but it will probably not be a common
   requirement and so is currently put off to a rainy day. */
   if( !bnd_this && !bnd_that && astOK ) {
      astError( AST__INTER, "astOverlap(Region): Neither of the two "
                "supplied Regions (classes %s and %s) has a finite "
                "boundary.", astGetClass(this), astGetClass(that) );
      astError( AST__INTER, "The current implementation of astOverlap "
                "cannot determine the overlap between two Regions "
                "unless at least one of them has a finite boundary." );
   }

/* If only one of the two Regions has a finite boundary, we must use its
   mesh first. Choose the finite boundary Region as the "second" region.
   Also store a flag indicating if the first Region has a finite boundary. */
   if( bnd_that ) {
      reg1 = this;
      reg2 = that;
      bnd1 = bnd_this;
   } else {
      reg1 = that;
      reg2 = this;
      bnd1 = bnd_that;
   }

/* Get a FrameSet which connects the Frame represented by the second Region 
   to the Frame represented by the first Region. Check that the conection is 
   defined. */
   fs = astConvert( reg2, reg1, "" );
   if( fs ) {

/* Get a pointer to the Frame represented by the first Region. */
      frm_reg1 = astGetFrame( reg1->frameset, AST__CURRENT );

/* Get a pointer to the Mapping from current to base Frame in the first Region. */
      map_reg1 = astGetMapping( reg1->frameset, AST__CURRENT, AST__BASE );

/* Get the Mapping from the current Frame of the second Region to the
   current Frame of the first Region. */
      cmap = astGetMapping( fs, AST__BASE, AST__CURRENT );

/* Combine these Mappings to get the Mapping from current Frame of the
   second region to the base Frame of the first Region. */
      map = (AstMapping *) astCmpMap( cmap, map_reg1, 1, "" );

/* Get a mesh of points covering the second Region. These points are
   within the current Frame of the second Region. */
      reg2_mesh = astRegMesh( reg2 );

/* Transform this mesh into the base Frame of the first Region. */
      ps1 = astTransform( map, reg2_mesh, 1, NULL );

/* Also transform the Region describing the positional uncertainty within 
   the second supplied Region into the base Frame of the first supplied 
   Region. */
      unc = astGetUnc( reg2, AST__CURRENT );
      unc1 = astMapRegion( unc, map, frm_reg1 );

/* See if all points within this transformed mesh fall on the boundary of
   the first Region, to within the joint uncertainty of the two Regions. If
   so the two Regions have equivalent boundaries. */
      if( astRegPins( reg1, ps1, unc1, NULL ) ) {

/* If the boundaries are equivalent, the Regions are either identical or 
   are mutually exclusive. To distinguish between these cases, we
   looked at the Bounded attributes. If the Bounded attribute is the same
   for both Regions then they are identical, otherwise they are mutually
   exclusive. */
         result = ( astGetBounded( reg1 ) == astGetBounded( reg2 ) ) ? 5 : 6;

/* If the boundaries of the two Regions are not equivalent. */
      } else {

/* Transform the points in the mesh covering the second Region into the
   current Frame of the first Region. */
         astAnnul( ps1 );
         ps1 = astTransform( cmap, reg2_mesh, 1, NULL );

/* Transform this mesh using the first Region as a Mapping. Any points
   outside the first region will be set bad in the output PointSet. */
         ps2 = astTransform( (AstMapping *) reg1, ps1, 1, NULL );

/* Get the number of axes and points in this PointSet. */
         nc = astGetNcoord( ps2 );
         np = astGetNpoint( ps2 );

/* Get pointers to the axis data in this PointSet, and check they can be
   used safely. */
         ptr = astGetPoints( ps2 );
         if( astOK ) {

/* Loop round all points checking if the axis values are bad. We want a
   flag saying if there are any good axis values and another flag saying if
   there are any bad axis values. */
            allbad = 1;
            allgood = 1;
            for( iax = 0; iax < nc; iax++ ) {
               p = ptr[ iax ];
               for( ip = 0; ip < np; ip++,p++ ) {
                  if( *p == AST__BAD ) {
                     allgood = 0;
                     if( !allbad ) break;
                  } else {
                     allbad = 0;
                     if( !allgood ) break;
                  }
               }
            }

/* If the entire mesh of the second Region was inside the first region,
   return a result depending on whether the second region is bounded or
   infinite. */
            if( allgood ) {
               result = astGetBounded( reg2 ) ?  3 : 4;

/* If part of the mesh of the second Region was inside the first region,
   and part was outside, return a result of 4 (partial overlap). */
            } else if( !allbad ) {
               result = 4;

/* Otherwise,(i.e. if all points in the second Region mesh were outside the 
   first Region) if the first region is unbounded then all points on the 
   second region mesh must be in a hole. The returned result then 
   depends on whether the second Region is bounded or not. */
            } else if( !astGetBounded( reg1 ) ) {
               result = astGetBounded( reg2 ) ? 1 : 2;

/* We now know that the first Region is bounded. This means it must have
   a finite boundary. Perform a sanity check that this is what we expected. 
   Passing this test ensures that it is safe to call astRegMesh on the
   first Region. */
            } else if( !bnd1 ) {
               if( astOK ) {
                  astError( AST__INTER, "astOverlap(Region): The first "
                            "Region is bounded but has not got a finite "
                            "boundary (AST programming error)." );
               }

/* Since the first Region is bounded, we have two possibilities - the boundary
   of the first Region is either entirely inside or entirely outside the 
   second Region. To distinguish between these two cases we get a mesh of 
   points covering the boundary of the first Region (now known to be finite) 
   and transform it using the second Region. */
            } else {
               reg1_mesh = astRegMesh( reg1 );

/* Transform this mesh into the coordinate system of the second Region. */
               ps3 = astTransform( cmap, reg1_mesh, 0, NULL );

/* Transform the points in this mesh using the second Region as a Mapping. 
   Any points outside the second region will be set bad in the output 
   PointSet. */
               ps4 = astTransform( (AstMapping *) reg2, ps3, 1, NULL );

/* Get pointers to the axis data in this PointSet,and check they can be
   used safely. */
               ptr = astGetPoints( ps4 );
               if( astOK ) {

/* If the first point in the mesh is inside the second Region, then all
   points are inside. Otherwise all points are outside. */
                  result =  ( ptr[ 0 ][ 0 ] == AST__BAD ) ? 1 : 2;

               }

/* Free resources.*/
               ps3 = astAnnul( ps3 );
               ps4 = astAnnul( ps4 );
               reg1_mesh = astAnnul( reg1_mesh );
            }
         }

/* Free resources.*/
         ps2 = astAnnul( ps2 );
      }

/* Free resources.*/
      fs = astAnnul( fs );
      frm_reg1 = astAnnul( frm_reg1 );
      map_reg1 = astAnnul( map_reg1 );
      cmap = astAnnul( cmap );
      map = astAnnul( map );
      ps1 = astAnnul( ps1 );
      reg2_mesh = astAnnul( reg2_mesh );
      unc = astAnnul( unc );
      unc1 = astAnnul( unc1 );
   }

/* The returned value should take account of whether "this" or "that" is
   the first Region. If "this" was used as the first Region, then the
   result value calculated above is already correct. If "that" was used as 
   the first Region, then we need to change the result to swap "this" and
   "that". */
   if( reg1 == that ) {
      if( result == 2 ) {
         result = 3;
      } else if( result == 3 ) {
         result = 2;
      }
   }

/* If not OK, return zero. */
   if( !astOK ) result = 0;

/* Return the result. */
   return result;
}

static void Overlay( AstFrame *template_frame, const int *template_axes,
                     AstFrame *result ) {
/*
*  Name:
*     Overlay

*  Purpose:
*     Overlay the attributes of a template Region on to another Frame.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void Overlay( AstFrame *template, const int *template_axes,
*                   AstFrame *result )

*  Class Membership:
*     Region member function (over-rides the protected astOverlay
*     method inherited from the Frame class).

*  Description:
*     This function overlays attributes from the current Frame of a
*     Region on to another Frame, so as to over-ride selected
*     attributes of that second Frame. Normally only those attributes
*     which have been specifically set in the template will be
*     transferred. This implements a form of defaulting, in which a
*     Frame acquires attributes from the template, but retains its
*     original attributes (as the default) if new values have not
*     previously been explicitly set in the template.

*  Parameters:
*     template
*        Pointer to the template Region, for whose current Frame
*        values should have been explicitly set for any attribute
*        which is to be transferred.
*     template_axes
*        Pointer to an array of int, with one element for each axis of
*        the "result" Frame (see below). For each axis in the result
*        frame, the corresponding element of this array should contain
*        the (zero-based) index of the axis in the current Frame of
*        the template Region to which it corresponds. This array is
*        used to establish from which template Frame axis any
*        axis-dependent attributes should be obtained.
*
*        If any axis in the result Frame is not associated with a
*        template Frame axis, the corresponding element of this array
*        should be set to -1.
*     result
*        Pointer to the Frame which is to receive the new attribute values.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the current Frame in the Region and invoke its 
   astOverlay method to overlay its attributes. Annul the Frame pointer
   afterwards. */
   fr = astGetFrame( ((AstRegion *) template_frame)->frameset, AST__CURRENT );
   astOverlay( fr, template_axes, result );
   fr = astAnnul( fr );
}

static void PermAxes( AstFrame *this_frame, const int perm[] ) {
/*
*  Name:
*     PermAxes

*  Purpose:
*     Permute the order of a Region's axes.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void PermAxes( AstFrame *this, const int perm[] )

*  Class Membership:
*     Region member function (over-rides the astPermAxes method
*     inherited from the Frame class).

*  Description:
*     This function permutes the order in which the axes in the
*     current Frame of a Region occur.

*  Parameters:
*     this
*        Pointer to the Region.
*     perm
*        An array of int (with one element for each axis of the
*        Region's current Frame) which lists the axes in their new
*        order. Each element of this array should be a (zero-based)
*        axis index identifying the axes according to their old
*        (un-permuted) order.

*  Notes:
*     - Only genuine permutations of the axis order are permitted, so
*     each axis must be referenced exactly once in the "perm" array.
*     - If more than one axis permutation is applied to the same Frame
*     in a Region, the effects are cumulative.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   AstPermMap *map;              /* Pointer to axis permutation Mapping */
   int *invperm;                 /* Pointer to inverse permutation array */
   int axis;                     /* Loop counter for axes */
   int naxes;                    /* Number of Region axes */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Validate the permutation array, to check that it describes a
   genuine permutation. */
   astCheckPerm( this, perm, "astPermAxes" );

/* Obtain a pointer to the Region's current Frame and invoke this
   Frame's astPermAxes method to permute its axes. Annul the Frame
   pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astPermAxes( fr, perm );
   fr = astAnnul( fr );

/* Obtain the number of axes in the Region's current Frame and allocate
   memory to hold an inverse permutation array. */
   naxes =  astGetNaxes( this );
   invperm = astMalloc( sizeof( int ) * (size_t) naxes );

/* Fill the inverse permutation array with values that will invert the
   axis permutation supplied. */
   if ( astOK ) {
      for ( axis = 0; axis < naxes; axis++ ) invperm[ perm[ axis ] ] = axis;

/* Create a PermMap that will permute coordinate values in the same way as
   the current Frame's axes have been permuted. */
      map = astPermMap( naxes, invperm, naxes, perm, NULL, "" );

/* Modify the Frame's relationship to the base Frame in the Region so that 
   the correct coordinate values remain associated with the permuted axes. */
      astRemapFrame( this->frameset, AST__CURRENT, map );

/* Annul the PermMap and free the inverse permutation array. */
      map = astAnnul( map );
   }
   invperm = astFree( invperm );
}

static AstFrame *PickAxes( AstFrame *this_frame, int naxes, const int axes[],
                           AstMapping **map ) {
/*
*  Name:
*     PickAxes

*  Purpose:
*     Create a new Frame by picking axes from a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstFrame *PickAxes( AstFrame *this, int naxes, const int axes[],
*                         AstMapping **map )

*  Class Membership:
*     Region member function (over-rides the astPickAxes protected
*     method inherited from the Frame class).

*  Description:
*     This function creates a new Frame whose axes are copies of axes
*     picked from the encapsulated Frame of an existing Region. Other
*     Frame attributes are also copied from this existing Frame to the
*     new Frame. Zero or more of the original axes may be picked in
*     any order, but each can be used only once. Additional axes (with
*     default characteristics) may be included in the new Frame if
*     required.
*
*     Optionally, a Mapping that converts between the original Frame's
*     axes and those of the new Frame may also be returned.

*  Parameters:
*     this
*        Pointer to the Region.
*     naxes
*        The number of axes required in the new Frame.
*     axes
*        Pointer to an array of int with naxes elements. This should
*        contain (zero based) axis indices specifying the axes which
*        are to be included in the new Frame, in the order
*        required. Each axis index may occur only once.
*
*        If additional (default) axes are also to be included, the
*        corresponding elements of this array should be set to -1.
*     map
*        Address of a location to receive a pointer to a new
*        Mapping. This will be a PermMap (or a UnitMap as a special
*        case) that describes the axis permutation that has taken
*        place between the current Frame of the Region and the new
*        Frame.  The forward transformation will convert from the
*        original Region's axes to the new one's, and vice versa.
*
*        If this Mapping is not required, a NULL value may be supplied
*        for this parameter.

*  Returned Value:
*     Pointer to the new Frame.

*  Notes:
*     - The class of object returned may differ from that of the
*     original current Frame, depending on which axes are
*     selected. For example, if a single axis is picked from a
*     SkyFrame (which always has two axes), the resulting Frame cannot
*     be a valid SkyFrame, so will revert to the parent class (Frame)
*     instead.
*     - The new Frame contains a deep copy of all the data selected
*     from the original current Frame. Modifying the new Frame will
*     therefore not affect the Region or the Frames it contains.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *frame;              /* Pointer to Frame to be returned */
   AstRegion *this;              /* Pointer to Region structure */

/* Initialise the returned pointers. */
   if ( map ) *map = NULL;
   frame = NULL;

/* Check the global error status. */
   if ( !astOK ) return frame;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Check that a valid set of axes is being selected . */
   astValidateAxisSelection( this, naxes, axes, "astPickAxes" );

/* Obtain a pointer to the Region's encapsulated FrameSet and use its
   astPickAxes method to obtain the required new Frame and Mapping. */
   frame = astPickAxes( this->frameset, naxes, axes, map );

/* If an error occurred, annul the Mapping pointer (if requested) and
   the new Frame pointer. */
   if ( !astOK ) {
      if ( map ) *map = astAnnul( *map );
      frame = astAnnul( frame );
   }

/* Return the pointer to the new Frame. */
   return frame;
}

static void RegBaseBox( AstRegion *this, double *lbnd, double *ubnd ){
/*
*+
*  Name:
*     astRegBaseBox

*  Purpose:
*     Returns the bounding box of an un-negated Region in the base Frame of 
*     the encapsulated FrameSet.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     void astRegBaseBox( AstRegion *this, double *lbnd, double *ubnd )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns the upper and lower axis bounds of a Region in 
*     the base Frame of the encapsulated FrameSet, assuming the Region
*     has not been negated. That is, the value of the Negated attribute
*     is ignored.

*  Parameters:
*     this
*        Pointer to the Region.
*     lbnd
*        Pointer to an array in which to return the lower axis bounds
*        covered by the Region in the base Frame of the encpauslated
*        FrameSet. It should have at least as many elements as there are 
*        axes in the base Frame.
*     ubnd
*        Pointer to an array in which to return the upper axis bounds
*        covered by the Region in the base Frame of the encapsulated
*        FrameSet. It should have at least as many elements as there are 
*        axes in the base Frame.

*-
*/

/* Check the inherited status. */
   if( !astOK ) return;

/* This abstract implementation simply reports an error. All sub-classes of 
   Region should over-ride this to return appropriate values. */
   astError( AST__INTER, "astRegBaseBox(%s): The %s class does not implement "
             "the astRegBaseBox method inherited from the Region class "
             "(internal AST programming error).", astGetClass( this ), 
             astGetClass( this ) );
}

static AstPointSet *RegBaseMesh( AstRegion *this ){
/*
*+
*  Name:
*     astRegBaseMesh

*  Purpose:
*     Return a PointSet containing points spread around the boundary of a 
*     Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstPointSet *astRegBaseMesh( AstRegion *this )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a PointSet containing a set of points on the
*     boundary of the Region. The points refer to the base Frame of
*     the encapsulated FrameSet.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     Pointer to the PointSet. The axis values in this PointSet will have 
*     associated accuracies derived from the uncertainties which were
*     supplied when the Region was created.
*
*    If the Region has no boundary (i.e. is equivalent to a NullRegion), the 
*    returned PointSet will contain a single point with a value of AST__BAD 
*    on every axis.

*  Notes:
*    - A NULL pointer is returned if an error has already occurred, or if
*    this function should fail for any reason.
*-
*/

/* Check the local error status. */
   if ( !astOK ) return NULL;

/* This abstract method must be over-ridden by each concrete sub-class.
   Report an error if this null imlementation is called.*/
   astError( AST__INTER, "astRegBaseMesh(%s): The %s class does not implement "
             "the astRegBaseMesh method inherited from the Region class "
             "(internal AST programming error).", astGetClass( this ), 
             astGetClass( this ) );
   return NULL;
}

static double *RegCentre( AstRegion *this, double *cen, double **ptr, 
                          int index, int ifrm ){
/*
*+
*  Name:
*     astRegCentre

*  Purpose:
*     Re-centre a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     double *astRegCentre( AstRegion *this, double *cen, double **ptr, 
*                           int index, int ifrm )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function shifts the centre of the supplied Region to a
*     specified position, or returns the current centre of the Region.

*  Parameters:
*     this
*        Pointer to the Region.
*     cen
*        Pointer to an array of axis values, giving the new centre.
*        Supply a NULL value for this in order to use "ptr" and "index" to 
*        specify the new centre.
*     ptr
*        Pointer to an array of points, one for each axis in the Region.
*        Each pointer locates an array of axis values. This is the format
*        returned by the PointSet method astGetPoints. Only used if "cen"
*        is NULL.
*     index
*        The index of the point within the arrays identified by "ptr" at
*        which is stored the coords for the new centre position. Only used 
*        if "cen" is NULL.
*     ifrm
*        Should be AST__BASE or AST__CURRENT. Indicates whether the centre 
*        position is supplied and returned in the base or current Frame of 
*        the FrameSet encapsulated within "this".

*  Returned Value:
*     If both "cen" and "ptr" are NULL then a pointer to a newly
*     allocated dynamic array is returned which contains the centre
*     coords of the Region. This array should be freed using astFree when
*     no longer needed. If either of "ptr" or "cen" is not NULL, then a
*     NULL pointer is returned.

*  Notes:
*    - Some Region sub-classes do not have a centre. Such classes will report 
*    an AST__INTER error code if this method is called.
*-
*/

/* Local Variables: */
   double *result;

/* Initialise */
   result = NULL;

/* Check the local error status. */
   if ( !astOK ) return result;

/* This abstract method must be over-ridden by each concrete sub-class
   which allows the centre to be shifted. Report an error if this null 
   imlementation is called.*/
   astError( AST__INTER, "astRegCentre(%s): The %s class does not implement "
             "the astRegCentre method inherited from the Region class "
             "(internal AST programming error).", astGetClass( this ), 
             astGetClass( this ) );

   return NULL;
}


static AstPointSet *RegMesh( AstRegion *this ){
/*
*+
*  Name:
*     astRegMesh

*  Purpose:
*     Return a PointSet containing points spread over the boundary of a 
*     Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstPointSet *astRegMesh( AstRegion *this )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a PointSet containing a mesh of points on the
*     boundary of the Region. The points refer to the current Frame of
*     the encapsulated FrameSet.

*  Parameters:
*     this
*        Pointer to the Region.

*  Returned Value:
*     Pointer to the PointSet. The axis values in this PointSet will have 
*     associated accuracies derived from the uncertainties which were
*     supplied when the Region was created. Annul the pointer using
*     astAnnul when it is no longer needed.

*  Notes:
*    - It should not be assumed that the returned points are evenly
*    spaced on the boundary.
*    - A NULL pointer is returned if an error has already occurred, or if
*    this function should fail for any reason.
*-
*/

/* Local Variables; */
   AstMapping *map;          /* Base -> current Frame Mapping */
   AstPointSet *result;      /* Pointer to returned PointSet */

/* Initialise the returned pointer */
   result = NULL;

/* Check the local error status. */
   if ( !astOK ) return result;

/* If the Region structure does not contain a pointer to a PointSet holding 
   positions evenly spread over the boundary of the Region in the base
   Frame, create one now. Note, we cannot cache the mesh in the current
   Frame in this way since the current Frame mesh depends on the proprties
   of the current Frame (e.g. System) which can be changed at any time. */
   if( !this->basemesh ) this->basemesh = astRegBaseMesh( this );

/* Get the simplified base->current Mapping */
   map = RegMapping( this );

/* If the Mapping is a UnitMap, just return a clone of the PointSet
   pointer stored in the Region structure. */
   if( astIsAUnitMap( map ) ){
      result = astClone( this->basemesh );

/* Otherwise, create a new PointSet holding the above points transformed 
   into the current Frame. */
   } else {
      result = astTransform( map, this->basemesh, 1, NULL );
   }

/* Free resources.*/
   map = astAnnul( map );

/* If an error has occurred, annul the returned PointSet. */
   if( !astOK ) result = astAnnul( result );

/* Return the result. */
   return result;
}

static int RegPins( AstRegion *this, AstPointSet *pset, AstRegion *unc, 
                    int **mask ){
/*
*+
*  Name:
*     astRegPins

*  Purpose:
*     Check if a set of points fall on the boundary of a given Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     int astRegPins( AstRegion *this, AstPointSet *pset, AstRegion *unc,
*                     int **mask )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns a flag indicating if the supplied set of
*     points all fall on the boundary of the given Region. 
*
*     Some tolerance is allowed, as specified by the uncertainty Region
*     stored in the supplied Region "this", and the supplied uncertainty
*     Region "unc" which describes the uncertainty of the supplied points.

*  Parameters:
*     this
*        Pointer to the Region.
*     pset
*        Pointer to the PointSet. The points are assumed to refer to the 
*        base Frame of the FrameSet encapsulated by "this".
*     unc
*        Pointer to a Region representing the uncertainties in the points
*        given by "pset". The Region is assumed to represent the base Frame 
*        of the FrameSet encapsulated by "this". Zero uncertainity is assumed 
*        if NULL is supplied.
*     mask
*        Pointer to location at which to return a pointer to a newly
*        allocated dynamic array of ints. The number of elements in this
*        array is equal to the value of the Npoint attribute of "pset".
*        Each element in the returned array is set to 1 if the
*        corresponding position in "pset" is on the boundary of the Region
*        and is set to zero otherwise. A NULL value may be supplied
*        in which case no array is created. If created, the array should
*        be freed using astFree when no longer needed.

*  Returned Value:
*     Non-zero if the points all fall on the boundary of the given
*     Region, to within the tolerance specified. Zero otherwise.

*-
*/

/* Check the inherited status. */
   if( !astOK ) return 0;

/* This abstract implementation simply reports an error. All sub-classes of 
   Region should over-ride this to return appropriate values. */
   astError( AST__INTER, "astRegPins(%s): The %s class does not implement "
             "the astRegPins method inherited from the Region class "
             "(internal AST programming error).", astGetClass( this ), 
             astGetClass( this ) );
   return 0;
}

static void RegCurBox( AstRegion *this, double *lbnd, double *ubnd ){
/*
*+
*  Name:
*     astRegCurBox

*  Purpose:
*     Returns the bounding box of an un-negated Region in the current Frame 
*     of the encapsulated FrameSet.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     void astRegCurBox( AstRegion *this, double *lbnd, double *ubnd )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function returns the upper and lower axis bounds of a Region in 
*     the current Frame of the encapsulated FrameSet, assuming the Region
*     has not been negated. That is, the value of the Negated attribute
*     is ignored.

*  Parameters:
*     this
*        Pointer to the Region.
*     lbnd
*        Pointer to an array in which to return the lower axis bounds
*        covered by the Region in the current Frame of the encapsulated
*        FrameSet. It should have at least as many elements as there are 
*        axes in the Region.
*     ubnd
*        Pointer to an array in which to return the upper axis bounds
*        covered by the Region in the current Frame of the encapsulated
*        FrameSet. It should have at least as many elements as there are 
*        axes in the Region.
*-
*/

/* Local Variables: */
   AstMapping *smap;          /* Simplified base -> current Mapping */
   double *lbndb;             /* Pointer to lower bounds on base box */
   double *ubndb;             /* Pointer to upper bounds on base box */
   int i;                     /* Axis count */
   int nbase;                 /* Number of base Frame axes */
   int ncur;                  /* Number of current Frame axes */

/* Check the inherited status. */
   if( !astOK ) return;

/* Find the number of axes in the base and current Frames of the
   encapsulated FrameSet. */
   nbase = astGetNin( this->frameset );
   ncur = astGetNout( this->frameset );

/* Get the bounding box in the base Frame of the encapsulated FrameSet. */
   lbndb = astMalloc( sizeof( double )*(size_t) nbase );   
   ubndb = astMalloc( sizeof( double )*(size_t) nbase );   
   astRegBaseBox( this, lbndb, ubndb );      

/* Get the simplified base to current Mapping. */
   smap = RegMapping( this );

/* Check pointers can be used safely. */
   if( astOK ) {

/* If the simplified Mapping is a UnitMap, just copy the base box bounds
   to the returned arrays */
      if( astIsAUnitMap( smap ) ) {
         for( i = 0; i < ncur; i++ ) {
            lbnd[ i ] = lbndb[ i ];
            ubnd[ i ] = ubndb[ i ];
         }

/* Otherwise, use astMapBox to find the corresponding current Frame
   limits. */ 
      } else {
         for( i = 0; i < ncur; i++ ) {
            astMapBox( smap, lbndb, ubndb, 1, i, lbnd + i, ubnd + i, 
                       NULL, NULL );
         }            
      }
   }

/* Release resources. */
   smap = astAnnul( smap );
   lbndb = astFree( lbndb );
   ubndb = astFree( ubndb );

}

static void RegOverlay( AstRegion *this, AstRegion *that ){
/*
*+
*  Name:
*     astRegOverlay

*  Purpose:
*     Copy properties from one Region to another.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     void astRegOverlay( AstRegion *this, AstRegion *that )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function copies selected properties from "that" to "this".
*     It is intended to be called by sub-classes which need to create a
*     similar copy of an existing Region. For instance, subclass
*     implementations of the Simplify method will usually use this
*     function to ensure that the simplified Region loooks like the original
*     Region.

*  Parameters:
*     this
*        Pointer to the new Region.
*     that
*        Pointer to the old Region.
*-
*/


/* Local Variables; */
   AstRegion *unc; 
   AstRegion *newunc; 

/* Check the inherited status. */
   if( !astOK ) return;

/* Copy the required attribute values. */
   this->meshsize = that->meshsize;
   this->negated = that->negated;
   this->closed = that->closed;
   this->regionfs = that->regionfs;

/* If the uncertainty Region in "that" is a default, erase any uncertainty 
   information in "this". Otherwise, simplify it (this may result in the
   uncertainty Region being expressed in the new base Frame of the parent
   Region. */
   if( !astTestUnc( that ) ) {
      astClearUnc( this );
   } else if( astTestUnc( this ) ) {
      unc = astGetUnc( this, AST__BASE );
      newunc = astSimplify( unc );
      if( newunc != unc ) {
         if( astIsACircle( newunc ) || astIsAEllipse( newunc ) || 
             astIsABox( newunc ) ) {
           astSetUnc( this, newunc );
         }
      }
      unc = astAnnul( unc );
      newunc = astAnnul( newunc );
   }     
}

static void ReportPoints( AstMapping *this_mapping, int forward,
                          AstPointSet *in_points, AstPointSet *out_points ) {
/*
*  Name:
*     ReportPoints

*  Purpose:
*     Report the effect of transforming a set of points using a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void ReportPoints( AstMapping *this, int forward,
*                        AstPointSet *in_points, AstPointSet *out_points )

*  Class Membership:
*     Region member function (over-rides the protected astReportPoints
*     method inherited from the Frame class).

*  Description:
*     This function reports the coordinates of a set of points before
*     and after being transformed by a Region, by writing them to
*     standard output.

*  Parameters:
*     this
*        Pointer to the Region.
*     forward
*        A non-zero value indicates that the Region's forward
*        coordinate transformation has been applied, while a zero
*        value indicates the inverse transformation.
*     in_points
*        Pointer to a PointSet which is associated with the
*        coordinates of a set of points before the Region was
*        applied.
*     out_points
*        Pointer to a PointSet which is associated with the
*        coordinates of the same set of points after the Region has
*        been applied.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_mapping;

/* Obtain a pointer to the Region's current Frame and invoke its
   astReportPoints method. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astReportPoints( (AstMapping *) fr, forward, in_points, out_points );
   fr = astAnnul( fr );

}

static void Resolve( AstFrame *this_frame, const double point1[],
                     const double point2[], const double point3[],
                     double point4[], double *d1, double *d2 ){
/*
*  Name:
*     Resolve

*  Purpose:
*     Resolve a vector into two orthogonal components

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void Resolve( AstFrame *this, const double point1[],
*                   const double point2[], const double point3[],
*                   double point4[], double *d1, double *d2 );

*  Class Membership:
*     Region member function (over-rides the protected astResolve
*     method inherited from the Frame class).

*  Description:
*     This function resolves a vector into two perpendicular components.
*     The vector from point 1 to point 2 is used as the basis vector.
*     The vector from point 1 to point 3 is resolved into components
*     parallel and perpendicular to this basis vector. The lengths of the
*     two components are returned, together with the position of closest
*     aproach of the basis vector to point 3.

*  Parameters:
*     this
*        Pointer to the Frame.
*     point1
*        An array of double, with one element for each Frame axis
*        (Naxes attribute). This marks the start of the basis vector,
*        and of the vector to be resolved.
*     point2
*        An array of double, with one element for each Frame axis
*        (Naxes attribute). This marks the end of the basis vector.
*     point3
*        An array of double, with one element for each Frame axis
*        (Naxes attribute). This marks the end of the vector to be
*        resolved.
*     point4
*        An array of double, with one element for each Frame axis
*        in which the coordinates of the point of closest approach of the
*        basis vector to point 3 will be returned.
*     d1
*        The address of a location at which to return the distance from
*        point 1 to point 4 (that is, the length of the component parallel
*        to the basis vector). Positive values are in the same sense as
*        movement from point 1 to point 2.
*     d2
*        The address of a location at which to return the distance from
*        point 4 to point 3 (that is, the length of the component
*        perpendicular to the basis vector). The value is always positive.

*  Notes:
*     - Each vector used in this function is the path of
*     shortest distance between two points, as defined by the
*     astDistance function.
*     - This function will return "bad" coordinate values (AST__BAD)
*     if any of the input coordinates has this value, or if the required
*     output values are undefined.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke this
   Frame's astResolve method. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astResolve( fr, point1, point2, point3, point4, d1, d2 );
   fr = astAnnul( fr );

}

static AstPointSet *ResolvePoints( AstFrame *this_frame, const double point1[],
                                   const double point2[], AstPointSet *in,
                                   AstPointSet *out ) {
/*
*  Name:
*     astResolvePoints

*  Purpose:
*     Resolve a set of vectors into orthogonal components

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstPointSet *astResolvePoints( AstFrame *this, const double point1[], 
*                                    const double point2[], AstPointSet *in,
*                                    AstPointSet *out )

*  Class Membership:
*     Region member function (over-rides the astResolvePoints method
*     inherited from the Frame class).

*  Description:
*     This function takes a Frame and a set of vectors encapsulated
*     in a PointSet, and resolves each one into two orthogonal components,
*     returning these two components in another PointSet.
*
*     This is exactly the same as the public astResolve method, except
*     that this method allows many vectors to be processed in a single call,
*     thus reducing the computational cost of overheads of many
*     individual calls to astResolve.

*  Parameters:
*     this
*        Pointer to the Frame.
*     point1
*        An array of double, with one element for each Frame axis
*        (Naxes attribute). This marks the start of the basis vector,
*        and of the vectors to be resolved.
*     point2
*        An array of double, with one element for each Frame axis
*        (Naxes attribute). This marks the end of the basis vector.
*     in
*        Pointer to the PointSet holding the ends of the vectors to be
*        resolved.
*     out
*        Pointer to a PointSet which will hold the length of the two
*        resolved components. A NULL value may also be given, in which 
*        case a new PointSet will be created by this function.

*  Returned Value:
*     Pointer to the output (possibly new) PointSet. The first axis will 
*     hold the lengths of the vector components parallel to the basis vector. 
*     These values will be signed (positive values are in the same sense as 
*     movement from point 1 to point 2. The second axis will hold the lengths 
*     of the vector components perpendicular to the basis vector. These
*     values will always be positive.

*  Notes:
*     - The number of coordinate values per point in the input
*     PointSet must match the number of axes in the supplied Frame.
*     - If an output PointSet is supplied, it must have space for
*     sufficient number of points and 2 coordinate values per point.
*     - A null pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstPointSet *result;          /* Pointer to output PointSet */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Initialise. */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke this
   Frame's astResolve method. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astResolvePoints( this, point1, point2, in, out );
   fr = astAnnul( fr );

/* Return a pointer to the output PointSet. */
   return result;

}

static void SetAttrib( AstObject *this_object, const char *setting ) {
/*
*  Name:
*     SetAttrib

*  Purpose:
*     Set an attribute value for a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void SetAttrib( AstObject *this, const char *setting )

*  Class Membership:
*     Region member function (extends the astSetAttrib method
*     inherited from the Frame class).

*  Description:
*     This function assigns an attribute value for a Region, the
*     attribute and its value being specified by means of a string of
*     the form:
*
*        "attribute= value "
*
*     Here, "attribute" specifies the attribute name and should be in
*     lower case with no white space present. The value to the right
*     of the "=" should be a suitable textual representation of the
*     value to be assigned and this will be interpreted according to
*     the attribute's data type.  White space surrounding the value is
*     only significant for string attributes.

*  Parameters:
*     this
*        Pointer to the Region.
*     setting
*        Pointer to a null terminated string specifying the new
*        attribute value.

*  Notes:
*     - This protected method is intended to be invoked by the Object
*     astSet method and makes additional attributes accessible to it.
*/

/* Local Variables: */
   AstRegion *this;              /* Pointer to the Region structure */
   double dval;                  /* Floating point attribute value */
   int ival;                     /* Integer attribute value */
   int id;                       /* Offset of ID string */
   int len;                      /* Length of setting string */
   int nc;                       /* Number of characters read by astSscanf */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_object;

/* Obtain the length of the setting string. */
   len = strlen( setting );

/* Test for each recognised attribute in turn, using "astSscanf" to parse the
   setting string and extract the attribute value (or an offset to it in the
   case of string values). In each case, use the value set in "nc" to check
   that the entire string was matched. Once a value has been obtained, use the
   appropriate method to set it. */

/* We first handle attributes that apply to the Region as a whole
   (rather than to the encapsulated Frame). */

/* Negated */
/* ------- */
   if ( nc = 0,
        ( 1 == astSscanf( setting, "negated= %d %n", &ival, &nc ) )
          && ( nc >= len ) ) {
      astSetNegated( this, ival );

/* Closed */
/*------- */
   } else if ( nc = 0,
        ( 1 == astSscanf( setting, "closed= %d %n", &ival, &nc ) )
          && ( nc >= len ) ) {
      astSetClosed( this, ival );

/* FillFactor */
/* ---------- */
   } else if ( nc = 0,
        ( 1 == astSscanf( setting, "fillfactor= %lg %n", &dval, &nc ) )
        && ( nc >= len ) ) {
      astSetFillFactor( this, dval );

/* MeshSize */
/* -------- */
   } else if ( nc = 0,
        ( 1 == astSscanf( setting, "meshsize= %d %n", &ival, &nc ) )
          && ( nc >= len ) ) {
      astSetMeshSize( this, ival );

/* Now do attributes inherited from parent classes. We do these here to
   avoid the settings being passed on to the encapsulated FrameSet below. */

/* ID. */
/* --- */
   } else if ( nc = 0, ( 0 == astSscanf( setting, "id=%n%*[^\n]%n", &id, &nc ) )
                       && ( nc >= len ) ) {
      astSetID( this, setting + id );

/* Ident. */
/* ------ */
   } else if ( nc = 0, ( 0 == astSscanf( setting, "ident=%n%*[^\n]%n", &id, &nc ) )
                       && ( nc >= len ) ) {
      astSetIdent( this, setting + id );

/* Invert. */
/* ------- */
   } else if ( nc = 0,
        ( 1 == astSscanf( setting, "invert= %d %n", &ival, &nc ) )
        && ( nc >= len ) ) {
      astSetInvert( this, ival );

/* Report. */
/* ------- */
   } else if ( nc = 0,
        ( 1 == astSscanf( setting, "report= %d %n", &ival, &nc ) )
        && ( nc >= len ) ) {
      astSetReport( this, ival );

/* Define macros to see if the setting string matches any of the
   read-only attributes of this class. */
#define MATCH(attrib) \
        ( nc = 0, ( 0 == astSscanf( setting, attrib "=%*[^\n]%n", &nc ) ) && \
                  ( nc >= len ) )

#define AXISMATCH(attrib) \
        ( nc = 0, ( 0 == astSscanf( setting, attrib "(%*d)=%*[^\n]%n", &nc ) ) && \
                  ( nc >= len ) )

/* If the attribute was not recognised, use this macro to report an error
   if a read-only attribute has been specified. */
   } else if ( MATCH( "class" ) ||
               MATCH( "nin" ) ||
               MATCH( "nobject" ) ||
               MATCH( "bounded" ) ||
               MATCH( "nout" ) ||
               MATCH( "refcount" ) ||
               MATCH( "tranforward" ) ||
               MATCH( "traninverse" ) ) {
      astError( AST__NOWRT, "astSet: The setting \"%s\" is invalid for a %s.",
                setting, astGetClass( this ) );
      astError( AST__NOWRT, "This is a read-only attribute." );

/* Pass unrecognised attributes on to the Region's encapsulated FrameSet for
   further interpretation. Do not pass on FrameSet attributes since we
   pretend to the outside world that the encapsulated FrameSet is actually a
   Frame. Use the public astSet method rather than the protected
   astSetAttrib method so that the current Frame int he encapsulated
   FrameSet will be re-mapped if the attribute changes require it. */
   } else if ( !MATCH( "base" ) &&
               !MATCH( "current" ) &&
               !MATCH( "nframe" ) ) {
      astSet( this->frameset, setting );
   }

/* Undefine macros local to this function. */
#undef MATCH
}

static void SetAxis( AstFrame *this_frame, int axis, AstAxis *newaxis ) {
/*
*  Name:
*     SetAxis

*  Purpose:
*     Set a new Axis for a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void SetAxis( AstFrame *this, int axis, AstAxis *newaxis )

*  Class Membership:
*     Region member function (over-rides the astSetAxis method
*     inherited from the Frame class).

*  Description:
*     This function allows a new Axis object to be associated with one
*     of the axes of the current Frame in a Region, replacing the
*     previous one. Each Axis object contains a description of the
*     quantity represented along one of the Frame's axes, so this
*     function allows this description to be exchanged for another
*     one.

*  Parameters:
*     this
*        Pointer to the Region.
*     axis
*        The index (zero-based) of the axis whose associated Axis
*        object is to be replaced.
*     newaxis
*        Pointer to the new Axis object.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index supplied. */
   (void) astValidateAxis( this, axis, "astSetAxis" );

/* Obtain a pointer to the Region's current Frame and invoke this
   Frame's astSetAxis method to assign the new Axis object. Annul the
   Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astSetAxis( fr, axis, newaxis );
   fr = astAnnul( fr );
}

static void SetRegFS( AstRegion *this, AstFrame *frm ) {
/*
*+
*  Name:
*     astSetRegFS

*  Purpose:
*     Stores a new FrameSet in a Region

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     void astSetRegFS( AstRegion *this, AsstFrame *frm )

*  Class Membership:
*     Region virtual function.

*  Description:
*     This function creates a new FrameSet and stores it in the supplied
*     Region. The new FrameSet contains two copies of the supplied
*     Frame, connected by a UnitMap.

*  Parameters:
*     this
*        Pointer to the Region.
*     frm
*        The Frame to use.
*-
*/

/* Local Variables: */
   AstFrame *f1;         /* Copy of supplied Frame */
   AstFrame *f2;         /* Copy of supplied Frame */
   AstFrameSet *fs;      /* New FrameSet */
   AstUnitMap *um;       /* UnitMap connecting base anc current Frames */

/* Check the global error status. */
   if ( !astOK ) return;

/* Create the new FrameSet. First two copies of the supplied Frame so
   that modifications using the supplied pointer will not affect the new
   FrameSet. W create two copies (rather than 1) because the base and
   current Frames must be independant objects - otherwise attribute changes
   done to one will also appear in the other. Then construct the FrameSet 
   containing the two Frame copies connected by a UnitMap. */
   f1 = astCopy( frm );
   f2 = astCopy( frm );
   fs = astFrameSet( f1, "" );
   um = astUnitMap( astGetNaxes( f1 ), "" );
   astAddFrame( fs, AST__BASE, um, f2 );
   um = astAnnul( um );
   f1 = astAnnul( f1 );
   f2 = astAnnul( f2 );

/* Annul any existing FrameSet */
   if( this->frameset ) astAnnul( this->frameset );

/* Use the new FrameSet */
   this->frameset = fs;

}

static void SetUnc( AstRegion *this, AstRegion *unc ){
/*
*++
*  Name:
c     astSetUnc
f     AST_SETUNC

*  Purpose:
*     Store uncertainty information in a Region.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "region.h"
c     void astSetUnc( AstRegion *this, AstRegion *unc )
f     CALL AST_SETUNC( THIS, UNC, STATUS )

*  Class Membership:
*     Region method.

*  Description:
*     Each Region (of any class) can have an "uncertainty" which specifies 
*     the uncertainties associated with the boundary of the Region. This
*     information is supplied in the form of a second Region. The uncertainty 
*     in any point on the boundary of a Region is found by shifting the 
*     associated "uncertainty" Region so that it is centred at the boundary 
*     point being considered. The area covered by the shifted uncertainty 
*     Region then represents the uncertainty in the boundary position. 
*     The uncertainty is assumed to be the same for all points.
*
*     The uncertainty is usually specified when the Region is created, but
*     this
c     function
f     routine
*     allows it to be changed at any time. 

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Region which is to be assigned a new uncertainty.
c     unc
f     UNC = INTEGER (Given)
*        Pointer to the new uncertainty Region. This must be either a Box, 
*        a Circle or an Ellipse. A deep copy of the supplied Region will be 
*        taken, so subsequent changes to the uncertainty Region using the 
*        supplied pointer will have no effect on the Region 
c        "this".
f        THIS.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*--
*/

/* Local Variables: */
   AstFrame *frm;           /* Current Frame from FrameSet */
   AstFrameSet *fs2;        /* FrameSet from "unc" current Frame to "this" base Frame */
   AstFrameSet *fs;         /* FrameSet in "this" supplied Region */
   AstMapping *map2;        /* Base->current Mapping from FrameSet */
   AstMapping *map;         /* Base->current Mapping from FrameSet */
   AstMapping *smap;        /* Simplified base->current Mapping */
   double **ptr_reg;        /* Pointer to axis values for Region's Pointset */

/* Check the inherited status. */
   if( !astOK ) return;

/* Annul any existing uncertainty Region. */
   if( this->unc && astIsAObject( this->unc ) ) this->unc = astAnnul( this->unc );
   this->defunc = 1;

/* Check an uncertainty Region was supplied, and is of a usable class. */
   if( unc && ( astIsABox( unc ) || astIsACircle( unc ) || 
                astIsAEllipse( unc ) ) ){

/* Map it into the same Frame as that represented by the base Frame in 
   the supplied Region. */
      fs = this->frameset;
      astInvert( fs );
      fs2 = Conv( unc->frameset, fs );
      astInvert( fs );

      if( fs2 ) {
         map = astGetMapping( fs2, AST__BASE, AST__CURRENT );
         frm = astGetFrame( fs2, AST__CURRENT );
         this->unc = astMapRegion( unc, map, frm );

/* Ensure the Region is bounded. We know that negating an unbounded
   Region will make it bounded because we know that the Region is a
   Circle, Box or Ellipse, all of which have this property. */
         if( !astGetBounded( this->unc ) ) astNegate( this->unc );

/* Indicate that the uncertainty is not a default.*/
         this->defunc = 0;         

/* If the base Frame in the uncertainty Region is the same as the base
   Frame in the Region being dumped, then we do no need to include the
   FrameSet in the dump of the uncertainty Region. Since the current
   Frame in the uncertainty Region always corresponds to the base Frame of
   its parent Region, we only need to check if the base->current Mapping
   in the uncertainty Region's FrameSet is a UnitMap or not (after 
   simplification). If it is, set the RegionFS attribute of the uncertainty 
   Region to zero (i.e. false). This will cause the FrameSet to be omitted 
   from the Dump. */
         map2 = astGetMapping( this->unc->frameset, AST__BASE, AST__CURRENT );
         smap = astSimplify( map2 );
         if( astIsAUnitMap( smap ) ) astSetRegionFS( this->unc, 0 );

/* Re-centre the uncertainty Region at the first position in the PointSet
   associated with the Region structure. */
         ptr_reg = astGetPoints( this->points );
         astRegCentre( this->unc, NULL, ptr_reg, 0, AST__CURRENT );

/* Free resources */
         map2 = astAnnul( map2 );
         frm = astAnnul( frm );
         fs2 = astAnnul( fs2 );
         smap = astAnnul( smap );
         map = astAnnul( map );

/* Report error if conversion between Frames is not possible. */
      } else if( astOK ) {
         astError( AST__BADIN, "astSetUnc(%s): Bad %d dimensional "
                   "uncertainty Frame (%s %s) supplied.", astGetClass(this), 
                   astGetNaxes(unc), astGetDomain(unc), astGetTitle(unc) );
         astError( AST__NCPIN, "Cannot convert it to the Frame of the "
                   "new %s.", astGetClass( this ) );
      }      

/* Report an error if it is not of a usable class. */
   } else if( unc && astOK ){
      astError( AST__BADIN, "astSetUnc(%s): Bad uncertainty shape "
                "(%s) supplied.", astGetClass( this ), astGetClass(unc) );
      astError( AST__NCPIN, "The uncertainty must be a Box, Circle or Ellipse." );
   }
}

static AstMapping *Simplify( AstMapping *this_mapping ) {
/*
*  Name:
*     Simplify

*  Purpose:
*     Simplify the Mapping represented by a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstMapping *Simplify( AstMapping *this )

*  Class Membership:
*     Region method (over-rides the astSimplify method inherited
*     from the Frame class).

*  Description:
*     This function simplifies the encapsulated FrameSet and any
*     uncertainty Region in the supplied Region. This is different to 
*     the Simplify method in the parent Frame class which always returns 
*     a UnitMap.

*  Parameters:
*     this
*        Pointer to the original Region.

*  Returned Value:
*     A pointer to the simplified Region. A cloned pointer to the
*     supplied Region will be returned if no simplication could be
*     performed.

*  Notes:
*     - This implementation just simplifies the encapsulated FrameSet
*     and uncertainty Region. Sub-classes should usually provide their own 
*     implementation which invokes this implemetation, and then continues to 
*     check for further simplifications (such as fitting a new region to the 
*     current Frame).
*     - A NULL pointer value will be returned if this function is
*     invoked with the AST error status set, or if it should fail for
*     any reason.
*/

/* Local Variables: */
   AstFrame *bfrm;               /* Pointer to "this" baseFrame */
   AstFrameSet *fs;              /* Pointer to encapsulated FrameSet */
   AstMapping *map;              /* Base->current Mapping for "this" */
   AstMapping *result;           /* Result pointer to return */
   AstPointSet *pset1;           /* Base Frame centre position */
   AstPointSet *pset2;           /* Current Frame centre position */
   AstRegion *new;               /* Pointer to simplified Region */
   AstRegion *sunc;              /* Simplified uncertainty Region */
   AstRegion *this;              /* Pointer to original Region structure */
   AstRegion *unc;               /* Original uncertainty Region */
   double **ptr1;                /* Pointer to axis values in "pset1" */
   double *cen;                  /* Original centre of uncertainty Region */
   double *lbnd;                 /* Lower bounds of "this" bounding box */
   double *orig_cen;             /* Original centre for uncertainty Region */
   double *s1_lbnd;              /* Lower bounds of "unc" when centred at lbnd */
   double *s1_ubnd;              /* Upper bounds of "unc" when centred at lbnd */
   double *s2_lbnd;              /* Lower bounds of "unc" when centred at ubnd */
   double *s2_ubnd;              /* Upper bounds of "unc" when centred at ubnd */
   double *ubnd;                 /* Upper bounds of "this" bounding box */
   double w1;                    /* Width of "s1" bounding box */
   double w2;                    /* Width of "s2" bounding box */
   int ic;                       /* Axis index */
   int naxb;                     /* No. of base Frame axes in "this" */
   int nin;                      /* Number of base Frame axes in "this" */
   int nout;                     /* Number of current Frame axes in "this" */
   int ok;                       /* Can we use the simplified uncertainty? */
   int simpler;                  /* Has some simplication taken place? */
    
/* Initialise. */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_mapping;

/* Take a deep copy of the supplied Region. This is so that the returned
   pointer will have a diferent value to the supplied pointer if any
   simplication takes place. */
   new = astCopy( this );

/* Simplify the encapsulated FrameSet, and note if any simplification took
   place. */
   fs = astSimplify( new->frameset );
   simpler = ( fs != new->frameset );

/* If so, annull the existing FrameSet and use the simpler FrameSet. */
   if( simpler ) {
      (void) astAnnul( new->frameset );
      new->frameset = astClone( fs );
   } 
   fs = astAnnul( fs );

/* If the Region has default uncertainty, we simplify the uncertainty
   Region simply by deleting it. It will be regenerated when needed,
   using the simplified Region. */
   if( !astTestUnc( new ) ){  
      astClearUnc( new );

/* If the Region's uncertainty was supplied explicitly, try simplifying
   the unncertainty Region. */
   } else {

/* Obtain the Region's uncertainty. */
      unc = astGetUnc( new, AST__BASE );

/* Get the base->current Mapping from "this". */
      map = astGetMapping( this->frameset, AST__BASE, AST__CURRENT );

/* If it has different numbers of inputs and outputs (e.g. a PermMap used
   to take a slice through a Region), we need to ensure that the
   uncertainty Region is centred on the slice. */
      nin = astGetNin( map );
      nout = astGetNout( map );
      if( nin != nout ) {

/* Get the current centre of the uncertainty Region in its current Frame
   (the same as the base Frame of "this"). */
         cen = astRegCentre( unc, NULL, NULL, 0, AST__CURRENT );

/* Store it in a PointSet so it can be transformed. */
         pset1 = astPointSet( 1, nin, "" );
         ptr1 = astGetPoints( pset1 );
         if( astOK ) for( ic = 0; ic < nin; ic++ ) ptr1[ ic ][ 0 ] = cen[ ic ];
    
/* Transform into the curent Frame of "this", and then back into the base
   Frame. */  
         pset2 = astTransform( map, pset1, 1, NULL );
         astTransform( map, pset2, 0, pset1 );

/* Re-centre the uncertainty Region at this position. */
         astRegCentre( unc, NULL, ptr1, 0, AST__CURRENT );

/* Free resources. */
         cen = astFree( cen );
         pset1 = astAnnul( pset1 );
         pset2 = astAnnul( pset2 );
      } 

/* Free resources. */
      map = astAnnul( map );

/* Try simplifying the uncertainty. Only proceed if the uncertainty can
   be simplified. */
      sunc = astSimplify( unc );
      if( sunc != unc ) {

/* If the uncertainty can be simplified it means that the base->current
   Mapping in the uncertainty Region is sufficiently linear to allow the
   uncertainty shape to retain its form when transformed from the base to
   the current Frane. But this has only been tested at the current centre 
   position in the uncertainty Region. The uncertainty Region should
   describe the whole of "this" Region, and so we need to check that the 
   simplified uncertainty does not change as we move it around within "this"
   Region. To do this, we re-centre the uncertainty region at opposite
   corners of the bounding box of "this", and then we find the bounding
   box of the re-centred uncertainty Region. If this uncertainty bounding 
   box changes from corner to corner, then we do not simplify the 
   uncertainty Region. First get the base Frame bounding box of "this". */
         naxb = astGetNin( this->frameset );
         lbnd = astMalloc( sizeof( double )*(size_t)naxb );      
         ubnd = astMalloc( sizeof( double )*(size_t)naxb );      
         astRegBaseBox( this, lbnd, ubnd );      

/* Note the original base Frame centre of the simplified uncertainty Region. */
         orig_cen = astRegCentre( sunc, NULL, NULL, 0, AST__BASE );

/* Re-centre it at the lower bounds of the bounding box of "this". This is
   in the base Frame of "this" which is the same as the current Frame of 
   "sunc". */
         astRegCentre( sunc, lbnd, NULL, 0, AST__CURRENT );

/* Get the bounding box of the re-centred uncertainty Region, within its
   current Frame, which is the same as the base Frame of "this". */
         s1_lbnd = astMalloc( sizeof( double )*(size_t)naxb );      
         s1_ubnd = astMalloc( sizeof( double )*(size_t)naxb );      
         astRegCurBox( sunc, s1_lbnd, s1_ubnd );      

/* Now re-centre the uncertainty Region at the upper bounds of the bounding 
   box of "this". */
         astRegCentre( sunc, ubnd, NULL, 0, AST__CURRENT );

/* Get the bounding box of the re-centred uncertainty Region. */
         s2_lbnd = astMalloc( sizeof( double )*(size_t)naxb );      
         s2_ubnd = astMalloc( sizeof( double )*(size_t)naxb );      
         astRegCurBox( sunc, s2_lbnd, s2_ubnd );      

/* Get a pointer to the base Frame of "this". */
         bfrm = astGetFrame( this->frameset, AST__BASE );

/* The "ok" flag is initialised to indicate that the simplfiied uncertainty
   Region should not be used. */
         ok = 0;

/* Check pointers can be referenced safely */
         if( astOK ) { 

/* Now indicate that the simplfiied uncertainty Region should be used. */
            ok = 1;

/* Loop round all axes of the base Frame of "this". */
            for( ic = 0; ic < naxb; ic++ ) {

/* Get the width of the two bounding boxes on this axis. */
               w1 = astAxDistance( bfrm, ic + 1, s1_lbnd[ ic ], s1_ubnd[ ic ] );
               w2 = astAxDistance( bfrm, ic + 1, s2_lbnd[ ic ], s2_ubnd[ ic ] );

/* If these differ by more than 0.1% then we determine that the simplified
   uncertainty Region varies in size across the bounding box of "this", and
   so we do not use the simplified uncertainty Region. The figure of 0.1%
   is arbitrary. */
               if( fabs( w1 - w2 ) > 0.005*( fabs( w1 ) + fabs( w2 ) ) ) {
                  ok = 0;
                  break;
               }
            }
         }

/* Reinstate the original base Frame centre of the simplified uncertainty Region. */
         astRegCentre( sunc, orig_cen, NULL, 0, AST__BASE );

/* Free resources. */
         orig_cen = astFree( orig_cen );
         lbnd = astFree( lbnd );
         ubnd = astFree( ubnd );
         s1_lbnd = astFree( s1_lbnd );
         s1_ubnd = astFree( s1_ubnd );
         s2_lbnd = astFree( s2_lbnd );
         s2_ubnd = astFree( s2_ubnd );
         bfrm = astAnnul( bfrm );

/* If we can use the simplified uncertainty Region, indicate that we have
   performed some simplification, and store the new uncertainty Region. */
         if( ok ) {
            simpler = 1;
            astSetUnc( new, sunc );
         }
      }

/* Free resources */
      unc = astAnnul( unc );
      sunc = astAnnul( sunc );
   }

/* If any simplification could be performed, return the new Region.
   Otherwise, return a clone of the supplied pointer. */
   if( simpler ){
      result = (AstMapping *) new;
   } else {
      new = astAnnul( new );
      result = astClone( this );
   }

/* If an error occurred, annul the returned pointer. */
   if ( !astOK ) result = astAnnul( result );

/* Return the result. */
   return result;
}

static int SubFrame( AstFrame *this_frame, AstFrame *template,
                     int result_naxes,
                     const int *target_axes, const int *template_axes,
                     AstMapping **map, AstFrame **result ) {
/*
*  Name:
*     SubFrame

*  Purpose:
*     Select axes from a Region and convert to the new coordinate system.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int SubFrame( AstFrame *target, AstFrame *template, int result_naxes,
*                   const int *target_axes, const int *template_axes,
*                   AstMapping **map, AstFrame **result )

*  Class Membership:
*     Region member function (over-rides the protected astSubFrame
*     method inherited from the Frame class).

*  Description:
*     This function selects a requested sub-set (or super-set) of the
*     axes from the current Frame of a "target" Region and creates a
*     new Frame with copies of the selected axes assembled in the
*     requested order. It then optionally overlays the attributes of a
*     "template" Frame on to the result. It returns both the resulting
*     Frame and a Mapping that describes how to convert between the
*     coordinate systems described by the current Frame of the target
*     Region and the result Frame. If necessary, this Mapping takes
*     account of any differences in the Frames' attributes due to the
*     influence of the template.

*  Parameters:
*     target
*        Pointer to the target Region, from whose current Frame the
*        axes are to be selected.
*     template
*        Pointer to the template Frame, from which new attributes for
*        the result Frame are to be obtained. Optionally, this may be
*        NULL, in which case no overlaying of template attributes will
*        be performed.
*     result_naxes
*        Number of axes to be selected from the target Region. This
*        number may be greater than or less than the number of axes in
*        the Region's current Frame (or equal).
*     target_axes
*        Pointer to an array of int with result_naxes elements, giving
*        a list of the (zero-based) axis indices of the axes to be
*        selected from the current Frame of the target Region. The
*        order in which these are given determines the order in which
*        the axes appear in the result Frame. If any of the values in
*        this array is set to -1, the corresponding result axis will
*        not be derived from the target Region, but will be assigned
*        default attributes instead.
*     template_axes
*        Pointer to an array of int with result_naxes elements. This
*        should contain a list of the template axes (given as
*        zero-based axis indices) with which the axes of the result
*        Frame are to be associated. This array determines which axes
*        are used when overlaying axis-dependent attributes of the
*        template on to the result. If any element of this array is
*        set to -1, the corresponding result axis will not receive any
*        template attributes.
*
*        If the template argument is given as NULL, this array is not
*        used and a NULL pointer may also be supplied here.
*     map
*        Address of a location to receive a pointer to the returned
*        Mapping.  The forward transformation of this Mapping will
*        describe how to convert coordinates from the coordinate
*        system described by the current Frame of the target Region
*        to that described by the result Frame. The inverse
*        transformation will convert in the opposite direction.
*     result
*        Address of a location to receive a pointer to the result Frame.

*  Returned Value:
*     A non-zero value is returned if coordinate conversion is
*     possible between the current Frame of the target Region and
*     the result Frame. Otherwise zero is returned and *map and
*     *result are returned as NULL (but this will not in itself result
*     in an error condition). In general, coordinate conversion should
*     always be possible if no template Frame is supplied but may not
*     always be possible otherwise.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to Region's current Frame */
   int match;                    /* Result to be returned */

/* Initialise. */
   *map = NULL;
   *result = NULL;
   match = 0;

/* Check the global error status. */
   if ( !astOK ) return match;

/* Invoke the parent astSubFrame method on the Frame represented by the
   region. */
   fr = astGetFrame( ((AstRegion *) this_frame)->frameset, AST__CURRENT );
   match = astSubFrame( fr, template, result_naxes, target_axes, template_axes,
                        map, result );
   fr = astAnnul( fr );

/* Return the result. */
   return match;
}

static AstSystemType SystemCode( AstFrame *this_frame, const char *system ) {
/*
*  Name:
*     SystemCode

*  Purpose:
*     Convert a string into a coordinate system type code.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     AstSystemType SystemCode( AstFrame *this, const char *system )

*  Class Membership:
*     Region member function (over-rides the protected astSystemCode
*     method inherited from the Frame class).

*  Description:
*     This function converts a string used for the external description of
*     a coordinate system into a Frame coordinate system type code (System
*     attribute value). It is the inverse of the astSystemString function.

*  Parameters:
*     this
*        Pointer to the Frame.
*     system
*        Pointer to a constant null-terminated string containing the
*        external description of the coordinate system.

*  Returned Value:
*     The System type code.

*  Notes:
*     - A value of AST__BADSYSTEM is returned if the coordinate system
*     description was not recognised. This does not produce an error.
*     - A value of AST__BADSYSTEM is also returned if this function
*     is invoked with the global error status set or if it should fail
*     for any reason.
*/

/* Local Variables: */
   AstSystemType result;      /* Result value to return */
   AstFrame *fr;              /* Pointer to FrameSet's current Frame */
   AstRegion *this;           /* Pointer to the Region structure */

/* Initialise. */
   result = AST__BADSYSTEM;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke the
   astSystemCode method for this Frame. Annul the Frame pointer afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astSystemCode( fr, system );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = AST__BADSYSTEM;

/* Return the result. */
   return result;
}

static const char *SystemString( AstFrame *this_frame, AstSystemType system ) {
/*
*  Name:
*     SystemString

*  Purpose:
*     Convert a coordinate system type code into a string.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     const char *SystemString( AstFrame *this, AstSystemType system )

*  Class Membership:
*     Region member function (over-rides the protected astSystemString
*     method inherited from the Frame class).

*  Description:
*     This function converts a Frame coordinate system type code
*     (System attribute value) into a string suitable for use as an
*     external representation of the coordinate system type.

*  Parameters:
*     this
*        Pointer to the Frame.
*     system
*        The coordinate system type code.

*  Returned Value:
*     Pointer to a constant null-terminated string containing the
*     textual equivalent of the type code supplied.

*  Notes:
*     - A NULL pointer value is returned if the coordinate system
*     code was not recognised. This does not produce an error.
*     - A NULL pointer value is also returned if this function is
*     invoked with the global error status set or if it should fail
*     for any reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to FrameSet's current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   const char *result;           /* Pointer value to return */

/* Initialise. */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke the
   astSystemString method for this Frame. Annul the Frame pointer 
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astSystemString( fr, system );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = NULL;

/* Return the result pointer. */
   return result;

}

static int TestAttrib( AstObject *this_object, const char *attrib ) {
/*
*  Name:
*     TestAttrib

*  Purpose:
*     Test if a specified attribute value is set for a Region.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int TestAttrib( AstObject *this, const char *attrib )

*  Class Membership:
*     Region member function (over-rides the astTestAttrib protected
*     method inherited from the Frame class).

*  Description:
*     This function returns a boolean result (0 or 1) to indicate whether
*     a value has been set for one of a Region's attributes.

*  Parameters:
*     this
*        Pointer to the Region.
*     attrib
*        Pointer to a null terminated string specifying the attribute
*        name.  This should be in lower case with no surrounding white
*        space.

*  Returned Value:
*     One if a value has been set, otherwise zero.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global status set, or if it should fail for any reason.
*/

/* Local Variables: */
   AstRegion *this;              /* Pointer to the Region structure */
   int result;                   /* Result value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_object;

/* Check the attribute name and test the appropriate attribute. */

/* We first handle attributes that apply to the Region as a whole
   (rather than to the encapsulated FrameSet). */

/* Negated. */
/* -------- */
   if ( !strcmp( attrib, "negated" ) ) {
      result = astTestNegated( this );

/* Closed. */
/* ------- */
   } else if ( !strcmp( attrib, "closed" ) ) {
      result = astTestClosed( this );

/* FillFactor */
/* ---------- */
   } else if ( !strcmp( attrib, "fillfactor" ) ) {
      result = astTestFillFactor( this );

/* MeshSize */
/* -------- */
   } else if ( !strcmp( attrib, "meshsize" ) ) {
      result = astTestMeshSize( this );

/* Now do attributes inherited from parent classes. This is so that the
   attribute test will not be passed on to the encpasulated FrameSet below. */

/* ID. */
/* --- */
   } else if ( !strcmp( attrib, "id" ) ) {
      result = astTestID( this );

/* Ident. */
/* ------ */
   } else if ( !strcmp( attrib, "ident" ) ) {
      result = astTestIdent( this );

/* Invert. */
/* ------- */
   } else if ( !strcmp( attrib, "invert" ) ) {
      result = astTestInvert( this );

/* Report. */
/* ------- */
   } else if ( !strcmp( attrib, "report" ) ) {
      result = astTestReport( this );

/* If the name is not recognised, test if it matches any of the
   read-only attributes of this class. If it does, then return
   zero. */
   } else if ( !strcmp( attrib, "class" ) ||
               !strcmp( attrib, "nin" ) ||
               !strcmp( attrib, "nobject" ) ||
               !strcmp( attrib, "bounded" ) ||
               !strcmp( attrib, "nout" ) ||
               !strcmp( attrib, "refcount" ) ||
               !strcmp( attrib, "tranforward" ) ||
               !strcmp( attrib, "traninverse" ) ) {
      result = 0;

/* Pass unrecognised attributes on to the Region's encapsulated FrameSet for
   further interpretation. Do not pass on FrameSet attributes since we
   pretend to the outside world that the encapsulated FrameSet is actually a
   Frame. */
   } else if ( strcmp( attrib, "base" ) &&
               strcmp( attrib, "current" ) &&
               strcmp( attrib, "nframe" ) ) {
      result = astTestAttrib( this->frameset, attrib );
   }

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = 0;

/* Return the result, */
   return result;
}

double *astRegTranPoint_( AstRegion *this, double *in, int np, int forward ){
/*
*+
*  Name:
*     astRegTranPoint

*  Purpose:
*     Transform points between the base and current Frames in a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     double *astRegTranPoint( AstRegion *this, double *in, int np, int forward )

*  Class Membership:
*     Region member function 

*  Description:
*     This function transforms one or more points between the base and
*     current Frames of the FrameSet encapsulated by the supplied Region.

*  Parameters:
*     this
*        The Region pointer.
*     in
*        Pointer to a 1-d array holding the axis values to be transformed.
*        If "forward" is non-zero, the number of axis values supplied for
*        each position should equal the number of axes in the base Frame
*        of the FrameSet encapsulated by "this". If "forward" is zero, the 
*        number of axis values supplied for each position should equal the 
*        number of axes in the current Frame of the FrameSet encapsulated by 
*        "this". All the axis values for a position should be in adjacent
*        elements of the array.
*     np
*        The number of points supplied in "in".
*     forward
*        If non-zero, the supplied points are assumed to refer to the base 
*        Frame of the encapsulated FrameSet, and they are transformed to the 
*        current Frame. If zero, the supplied points are assumed to refer to 
*        the current Frame of the encapsulated FrameSet, and they are 
*        transformed to the base Frame.

*  Returned Value:
*     Pointer to a new dynamically allocated array holding the
*     transformed axis values. If "forward" is non-zero, the number of axis 
*     values for each position will be equal the number of axes in the
*     current Frame of the FrameSet encapsulated by "this". If "forward" is 
*     zero, the number of axis values for each position will be equal to the 
*     number of axes in the base Frame of the FrameSet encapsulated by "this". 
*     All the axis values for a position will be in adjacent elements of the 
*     array. The array should be freed using astFree when no longer needed.

*  Notes:
*    - A NULL pointer is returned if an error has already occurred, or if
*    this function should fail for any reason.

*-
*/

/* Local Variables: */
   AstMapping *map;
   AstPointSet *pset_in;
   AstPointSet *pset_out;
   double **ptr_in;
   double **ptr_out;
   double *p;
   double *result; 
   int ic;           
   int ip;
   int naxin;
   int naxout;

/* Initialise */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Get a pointer to the required Mapping. */
   if( forward ) {
      map = astGetMapping( this->frameset, AST__BASE, AST__CURRENT );
   } else {
      map = astGetMapping( this->frameset, AST__CURRENT, AST__BASE );
   }

/* Get the number of axis values per input and per output point. */
   naxin = astGetNin( map );
   naxout = astGetNout( map );

/* Create a pointSet holding the supplied axis values. */
   pset_in = astPointSet( np, naxin, "" );

/* Get pointers to the memory used to store axis values within this
   PointSet. */
   ptr_in = astGetPoints( pset_in );

/* Allocate the output array. */
   result = astMalloc( sizeof( double )*(size_t)( naxout*np ) );

/* Check the pointers can be used. */
   if( astOK ) {

/* Store the supplied axis values in the PointSet memory. */
      p = in;
      for( ip = 0; ip < np; ip++ ) {
         for( ic = 0; ic < naxin; ic++ ) ptr_in[ ic ][ ip ] = *(p++);
      }

/* Transform the PointSet. */
      pset_out = astTransform( map, pset_in, 1, NULL );

/* Get a pointer to the memory in the transformed PointSet. */
      ptr_out = astGetPoints( pset_out );
      if( astOK ) {

/* Store the resulting axis values in the output array. */
         p = result;
         for( ip = 0; ip < np; ip++ ) {
            for( ic = 0; ic < naxout; ic++ ) *(p++) = ptr_out[ ic ][ ip ];
         }
      }

/* Free resources. */
      pset_out = astAnnul( pset_out );
   }
   pset_in = astAnnul( pset_in );
   map = astAnnul( map );

/* Return NULL if anything went wrong. */
   if( !astOK ) result = astAnnul( result );

/* Return the result.*/
   return result;
}

static AstPointSet *RegTransform( AstRegion *this, AstPointSet *in,
                                  int forward, AstPointSet *out, AstFrame **frm ) {
/*
*+
*  Name:
*     astRegTransform

*  Purpose:
*     Transform a set of points using the encapsulated FrameSet.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstPointSet *astRegTransform( AstRegion *this, AstPointSet *in,
*                                   int forward, AstPointSet *out,
*                                   AstFrameSet **frm )

*  Class Membership:
*     Region virtual function 

*  Description:
*     This function takes a Region and a set of points encapsulated
*     in a PointSet, and applies either the forward or inverse
*     coordinate transformation represented by the encapsulated FrameSet.
*     It also returned a pointer to either the current or base Frame in
*     the FrameSet.

*  Parameters:
*     this
*        Pointer to the Region.
*     in
*        Pointer to the PointSet holding the input coordinate data.
*     forward
*        A non-zero value indicates that the forward coordinate transformation
*        (from base to current) should be applied, while a zero value requests 
*        the inverse transformation (from current to base).
*     out
*        Pointer to a PointSet which will hold the transformed (output)
*        coordinate values. A NULL value may also be given, in which case a
*        new PointSet will be created by this function.
*     frm
*        Location at which to return a pointer to a Frame. If "forward"
*        is non-zero, the current Frame in the encapsulated FrameSet will
*        be returned. Otherwise, the base Frame is returned. The returned
*        pointer should be annulled when no longer needed. May be NULL if
*        no pointer is needed.

*  Returned Value:
*     Pointer to the output (possibly new) PointSet. If "out" is NULL,
*     the returned pointer will be a clone of "in" if the Mapping is a
*     UnitMap. If "out" is not NULL, then the supplied "out" PointSet will 
*     be used and returned.

*  Notes:
*     - An error will result if the Region supplied does not define
*     the requested coordinate transformation (either forward or
*     inverse).
*     - The number of coordinate values per point in the input
*     PointSet must match the number of input coordinates for the
*     Region being applied (or number of output coordinates if the
*     inverse transformation is requested).  This will be equal to the
*     number of axes in the Region's base Frame (or the current
*     Frame for the inverse transformation).
*     - If an output PointSet is supplied, it must have space for
*     sufficient number of points and coordinate values per point to
*     accommodate the result (e.g. the number of Region output
*     coordinates, or number of input coordinates if the inverse
*     transformation is requested). Any excess space will be ignored.
*     - A null pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   AstMapping *smap;           /* Pointer to simplified Mapping */
   AstPointSet *result;        /* Pointer value to return */

/* Initialise */
   if( frm ) *frm = NULL;

/* Check the global error status. */
   if ( !astOK ) return NULL;

/* Get the simplified Mapping from base to current Frame. */
   smap = RegMapping( this );

/* If it is a UnitMap, return a clone of the input PointSet unless an
   explicit output PointSet has been supplied. */
   if( astIsAUnitMap( smap ) && !out ) {
      result = astClone( in );

/* Otherwise use the Mapping to transform the supplied positions. */
   } else {
      result = astTransform( smap, in, forward, out );
   }

/* Return a pointer to the appropriate Frame. */
   if( frm ) *frm = astGetFrame( this->frameset, forward ? AST__CURRENT : AST__BASE );

/* Release resources. */
   smap = astAnnul( smap );

/* Return a pointer to the output PointSet. */
   return result;
}

static int Unformat( AstFrame *this_frame, int axis, const char *string,
                     double *value ) {
/*
*  Name:
*     Unformat

*  Purpose:
*     Read a formatted coordinate value for a Region axis.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int Unformat( AstFrame *this, int axis, const char *string,
*                   double *value )

*  Class Membership:
*     Region member function (over-rides the public astUnformat
*     method inherited from the Frame class).

*  Description:
*     This function reads a formatted coordinate value for a Region
*     axis (supplied as a string) and returns the equivalent numerical
*     value as a double. It also returns the number of characters read
*     from the string.

*  Parameters:
*     this
*        Pointer to the Region.
*     axis
*        The number of the Region axis for which the coordinate
*        value is to be read (axis numbering starts at zero for the
*        first axis).
*     string
*        Pointer to a constant null-terminated string containing the
*        formatted coordinate value.
*     value
*        Pointer to a double in which the coordinate value read will be
*        returned.

*  Returned Value:
*     The number of characters read from the string to obtain the
*     coordinate value.

*  Notes:
*     - Any white space at the beginning of the string will be
*     skipped, as also will any trailing white space following the
*     coordinate value read. The function's return value will reflect
*     this.
*     - A function value of zero (and no coordinate value) will be
*     returned, without error, if the string supplied does not contain
*     a suitably formatted value.
*     - The string "<bad>" is recognised as a special case and will
*     generate the value AST__BAD, without error. The test for this
*     string is case-insensitive and permits embedded white space.
*     - A function result of zero will be returned and no coordinate
*     value will be returned via the "value" pointer if this function
*     is invoked with the global error status set, or if it should
*     fail for any reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   double coord;                 /* Coordinate value read */
   int nc;                       /* Number of characters read */

/* Initialise. */
   nc = 0;

/* Check the global error status. */
   if ( !astOK ) return nc;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Validate the axis index. */
   (void) astValidateAxis( this, axis, "astUnformat" );

/* Obtain a pointer to the Region's current Frame and invoke the
   astUnformat method for this Frame. Annul the Frame pointer
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   nc = astUnformat( fr, axis, string, &coord );
   fr = astAnnul( fr );

/* If an error occurred, clear the number of characters read. */
   if ( !astOK ) {
      nc = 0;

/* Otherwise, if characters were read, return the coordinate value. */
   } else if ( nc ) {
      *value = coord;
   }
   
/* Return the number of characters read. */
   return nc;
}

static int ValidateAxis( AstFrame *this_frame, int axis, const char *method ) {
/*
*  Name:
*     ValidateAxis

*  Purpose:
*     Validate and permute a Region's axis index.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int ValidateAxis( AstFrame *this, int axis, const char *method )

*  Class Membership:
*     Region member function (over-rides the protected
*     astValidateAxis method inherited from the Frame class).

*  Description:
*     This function checks the validity of an index (zero-based) which
*     is to be used to address one of the coordinate axes of the
*     current Frame in a Region. If the index is valid, it is
*     permuted using the axis permutation array associated with the
*     Region's current Frame and the (zero-based) permuted axis
*     index is returned.  This gives the index the axis had when the
*     Frame was first created. If the axis index supplied is not
*     valid, an error is reported and the global error status is set.

*  Parameters:
*     this
*        Pointer to the Region.
*     axis
*        The axis index (zero-based) to be checked. To be valid, it
*        must lie between zero and (naxes-1) inclusive, where "naxes"
*        is the number of coordinate axes associated with the
*        Region's current Frame.
*     method
*        Pointer to a constant null-terminated character string
*        containing the name of the method that invoked this function
*        to validate an axis index. This method name is used solely
*        for constructing error messages.

*  Returned Value:
*     The permuted axis index.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */
   int naxes;                    /* Number of Region axes */
   int result;                   /* Permuted axis index */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_frame;

/* Determine the number of Region axes. */
   naxes = astGetNaxes( this );
   if ( astOK ) {

/* If the Region has no axes, report an error (convert to 1-based
   axis numbering for the benefit of the public interface). */
      if ( naxes == 0 ) {
         astError( AST__AXIIN, "%s(%s): Invalid attempt to use an axis index "
                   "(%d) for a %s which has no axes.", method,
                   astGetClass( this ), axis + 1, astGetClass( this ) );

/* Otherwise, check the axis index for validity and report an error if
   it is not valid (again, convert to 1-based axis numbering). */
      } else if ( ( axis < 0 ) || ( axis >= naxes ) ) {
         astError( AST__AXIIN, "%s(%s): Axis index (%d) invalid - it should "
                   "be in the range 1 to %d.", method, astGetClass( this ),
                   axis + 1, naxes );

/* If the axis index was valid, obtain a pointer to the Region's
   current Frame and invoke this Frame's astValidateAxis method to
   obtain the permuted axis index. Annul the Frame pointer
   afterwards. */
      } else {
         fr = astGetFrame( this->frameset, AST__CURRENT );
         result = astValidateAxis( fr, axis, "astValidateAxis" );
         fr = astAnnul( fr );
      }
   }

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = 0;

/* Return the result. */
   return result;
}

static void ValidateAxisSelection( AstFrame *this_frame, int naxes, 
                                   const int *axes, const char *method ) {
/*
*  Name:
*     ValidateAxisSelection

*  Purpose:
*     Check that a set of axes selected from a Frame is valid.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     void ValidateAxisSelection( AstFrame *this, int naxes,
*                                 const int *axes, const char *method )

*  Class Membership:
*     Region member function (over-rides the protected astValidateAxisSelection
*     method inherited from the Frame class).

*  Description:
*     This function checks the validity of an array of (zero-based)
*     axis indices that specify a set of axes to be selected from a
*     Frame. To be valid, no axis should be selected more than
*     once. In assessing this, any axis indices that do not refer to
*     valid Frame axes (e.g. are set to -1) are ignored.
*
*     If the axis selection is valid, this function returns without further
*     action. Otherwise, an error is reported and the global error status is
*     set.

*  Parameters:
*     this
*        Pointer to the Frame.
*     naxes
*        The number of axes to be selected (may be zero).
*     axes
*        Pointer to an array of int with naxes elements that contains the
*        (zero based) axis indices to be checked.
*     method
*        Pointer to a constant null-terminated character string
*        containing the name of the method that invoked this function
*        to validate an axis selection. This method name is used
*        solely for constructing error messages.
*/

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to current Frame */
   AstRegion *this;              /* Pointer to the Region structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke this
   Frame's astValidateAxisSelection method. Annul the Frame pointer 
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   astValidateAxisSelection( fr, naxes, axes, method );
   fr = astAnnul( fr );

}

static int ValidateSystem( AstFrame *this_frame, AstSystemType system, const char *method ) {
/*
*  Name:
*     ValidateSystem

*  Purpose:
*     Validate a value for a Frame's System attribute.

*  Type:
*     Private function.

*  Synopsis:
*     #include "region.h"
*     int ValidateSystem( AstFrame *this, AstSystemType system,
*                         const char *method )

*  Class Membership:
*     Region member function (over-rides the protected astValidateSystem
*     method inherited from the Frame class).

*  Description:
*     This function checks the validity of the supplied system value.
*     If the value is valid, it is returned unchanged. Otherwise, an
*     error is reported and a value of AST__BADSYSTEM is returned.

*  Parameters:
*     this
*        Pointer to the Frame.
*     system
*        The system value to be checked.
*     method
*        Pointer to a constant null-terminated character string
*        containing the name of the method that invoked this function
*        to validate an axis index. This method name is used solely
*        for constructing error messages.

*  Returned Value:
*     The validated system value.

*  Notes:
*     - A value of AST_BADSYSTEM will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   AstSystemType result;      /* Validated system value */
   AstFrame *fr;              /* Pointer to FrameSet's current Frame */
   AstRegion *this;           /* Pointer to the Region structure */

/* Initialise. */
   result = AST__BADSYSTEM;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the FrameSet structure. */
   this = (AstRegion *) this_frame;

/* Obtain a pointer to the Region's encapsulated Frame and invoke the
   astValidateSystem method for this Frame. Annul the Frame pointer 
   afterwards. */
   fr = astGetFrame( this->frameset, AST__CURRENT );
   result = astValidateSystem( this, system, method );
   fr = astAnnul( fr );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = AST__BADSYSTEM;

/* Return the result. */
   return result;
}

/* Region Attributes. */
/* -------------------- */

/*
*att++
*  Name:
*     Negated

*  Purpose:
*     Region negation flag.

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer (boolean).

*  Description:
*     This attribute controls whether a Region represents the "inside" or
*     the "outside" of the area which was supplied when the Region was
*     created. If the attribute value is zero (the default), the Region
*     represents the inside of the original area. However, if it is non-zero, 
*     it represents the outside of the original area. The value of this
*     attribute may be toggled using the 
c     astNegate function.
f     AST_NEGATE routine.

*     Note, whether the boundary is considered to be inside the Region or
*     not is controlled by the Closed attribute. Changing the value of
*     the Negated attribute does not change the value of the Closed attribute.
*     Thus, if Region is closed, then the boundary of the Region will be 
*     inside the Region, whatever the setting of the Negated attribute.

*  Applicability:
*     Region
*        All Regions have this attribute.
*att--
*/

/* This is a boolean value (0 or 1) with a value of -INT_MAX when
   undefined but yielding a default of zero. */
astMAKE_CLEAR(Region,Negated,negated,-INT_MAX)
astMAKE_GET(Region,Negated,int,0,( ( this->negated == -INT_MAX ) ?
                                   0 : this->negated ))
astMAKE_SET(Region,Negated,int,negated,( value != 0 ))
astMAKE_TEST(Region,Negated,( this->negated != -INT_MAX ))

/*
*att++
*  Name:
*     Bounded

*  Purpose:
*     Is the Region bounded?

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer (boolean), read-only.

*  Description:
*     This is a read-only attribute indicating if the Region is bounded.
*     A Region is bounded if it is contained entirely within some
*     finite-size bounding box.

*  Applicability:
*     Region
*        All Regions have this attribute.
*att--
*/

/*
*att+
*  Name:
*     RegionFS

*  Purpose:
*     Should Region FrameSet be dumped?

*  Type:
*     Protected attribute.

*  Synopsis:
*     Integer (boolean).

*  Description:
*     This attribute indicates whether the FrameSet encapsulated by the 
*     Region should be included in the dump produced by the Dump function.
*
*     If set to a non-zero value (the default), the FrameSet in the Region 
*     will be included in the dump as usual. If set to zero, the FrameSet 
*     will not be included in the dump. In this case, if the dump is 
*     subsequently used to re-create the Region, the new Region will have a 
*     default FrameSet containing a single default Frame with the appropriate 
*     number of axes. 
*
*     This facility is indended to reduce the size of textual dumps of
*     Regions in situations where the Frame to which the Region refers can
*     be implied by the context in which the Region is used. This is often the
*     case for instance when dumping the uncertainty Region encapsulated
*     by another parent Region - if the base Frame in the uncertainty Region 
*     is the same as the base Frame of the parent Region then there is no 
*     need to include a dump of the FramesSet when dumping the uncertainty 
*     Region.

*  Applicability:
*     Region
*        All Regions have this attribute.
*att-
*/

/* This is a boolean value (0 or 1) with a value of -INT_MAX when
   undefined but yielding a default of one. */
astMAKE_CLEAR(Region,RegionFS,regionfs,-INT_MAX)
astMAKE_GET(Region,RegionFS,int,1,( ( this->regionfs == -INT_MAX ) ?
                                   1 : this->regionfs ))
astMAKE_SET(Region,RegionFS,int,regionfs,( value != 0 ))
astMAKE_TEST(Region,RegionFS,( this->regionfs != -INT_MAX ))

/*
*att++
*  Name:
*     FillFactor

*  Purpose:
*     Fraction of the Region which is of interest.

*  Type:
*     Public attribute.

*  Synopsis:
*     Floating point.

*  Description:
*     This attribute indicates the fraction of the Region which is of
*     interest. AST does not use this attribute internally for any purpose.
*     Typically, it could be used to indicate the fraction of the Region for
*     which data is available.
*
*     The supplied value must be in the range 0.0 to 1.0, and the default
*     value is 1.0.

*  Applicability:
*     Region
*        All Regions have this attribute.
*att--
*/

astMAKE_CLEAR(Region,FillFactor,fillfactor,AST__BAD)
astMAKE_GET(Region,FillFactor,double,1.0,( ( this->fillfactor == AST__BAD ) ?
                                        1.0 : this->fillfactor ))
astMAKE_TEST(Region,FillFactor,( this->fillfactor != AST__BAD ))
astMAKE_SET(Region,FillFactor,double,fillfactor,((value<0.0||value>1.0)?(
       astError(AST__ATSER,"astSetFillFactor(%s): Invalid value (%g) supplied "
                "for attribute FillFactor.",astGetClass(this),value),
       astError(AST__ATSER,"FillFactor values should be in the range 0.0 to 1.0"), 
       this->fillfactor):value))

/* 
*att++
*  Name:
*     MeshSize

*  Purpose:
*     Number of points used to represent the boundary of a Region.

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer.

*  Description:
*     This attribute controls how many points are used when creating a
*     mesh of points covering the boundary of a Region. This mesh is used
*     primarily when testing for overlap with a second Region: each point in 
*     the mesh is checked to see if it is inside or outside the second Region.
*     Thus, the reliability of the overlap check depends on the value assigned 
*     to this attribute. If the value used is very low, it is possible for 
*     overlaps to go unnoticed. High values produce more reliable results, but 
*     can result in the overlap test being very slow. The default value is 200 
*     for two dimensional Regions and 2000 for three or more dimensional 
*     Regions (this attribute is not used for 1-dimensional regions since the 
*     boundary of a simple 1-d Region can only ever have two points). A
*     value of five is used if the supplied value is less than five.

*  Applicability:
*     Region
*        All Regions have this attribute.
*att--
*/
/* If the value of MeshSize is set or cleared, annul the PointSet used to
   cache a mesh of base Frame boundary points. This will force a new
   PointSet to be created next time it is needed. See function RegMesh. */
astMAKE_CLEAR(Region,MeshSize,meshsize,(
                 (this->basemesh)?(this->basemesh=astAnnul(this->basemesh)):0,
                 -INT_MAX))
astMAKE_SET(Region,MeshSize,int,meshsize,(
                 (this->basemesh)?(this->basemesh=astAnnul(this->basemesh)):0,
                 ( value > 5 ? value : 5 )))
astMAKE_TEST(Region,MeshSize,( this->meshsize != -INT_MAX ))
astMAKE_GET(Region,MeshSize,int,0,( ( this->meshsize == -INT_MAX)?((astGetNaxes(this)==1)?2:((astGetNaxes(this)==2)?200:2000)): this->meshsize ))

/*
*att++
*  Name:
*     Closed

*  Purpose:
*     Should the boundary be considered to be inside the region?

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer (boolean).

*  Description:
*     This attribute controls whether points on the boundary of a Region 
*     are considered to be inside or outside the region. If the attribute 
*     value is non-zero (the default), points on the boundary are considered 
*     to be inside the region (that is, the Region is "closed"). However, 
*     if the attribute value is zero, points on the bounary are considered
*     to be outside the region.

*  Applicability:
*     Region
*        All Regions have this attribute.
*     PointList
*        The value of the Closed attribute is ignored by PointList regions.
*        If the PointList region has not been negated, then it is always
*        assumed to be closed. If the PointList region has been negated, then 
*        it is always assumed to be open. This is required since points
*        have zero volume and therefore consist entirely of boundary.
*att--
*/
/* This is a boolean value (0 or 1) with a value of -INT_MAX when
   undefined but yielding a default of 1. */
astMAKE_CLEAR(Region,Closed,closed,-INT_MAX)
astMAKE_GET(Region,Closed,int,1,( ( this->closed == -INT_MAX ) ?
                                   1 : this->closed ))
astMAKE_SET(Region,Closed,int,closed,( value != 0 ))
astMAKE_TEST(Region,Closed,( this->closed != -INT_MAX ))

/* Access to attributes of the encapsulated Frame. */
/* ----------------------------------------------- */
/* Use the macros defined at the start of this file to implement
   private member functions that give access to the attributes of the
   encapsulated Frame of a Region and its axes. These functions over-ride
   the attribute access methods inherited from the Frame class. */

/* Clear, Get, Set and Test axis-independent Frame attributes. */
MAKE_CLEAR(Digits)
MAKE_CLEAR(Domain)
MAKE_CLEAR(MatchEnd)
MAKE_CLEAR(MaxAxes)
MAKE_CLEAR(MinAxes)
MAKE_CLEAR(Permute)
MAKE_CLEAR(PreserveAxes)
MAKE_CLEAR(Title)

MAKE_GET(Digits,int)
MAKE_GET(Domain,const char *)
MAKE_GET(MatchEnd,int)
MAKE_GET(MaxAxes,int)
MAKE_GET(MinAxes,int)
MAKE_GET(Permute,int)
MAKE_GET(PreserveAxes,int)
MAKE_GET(Title,const char *)
MAKE_SET(Digits,int,I)
MAKE_SET(Domain,const char *,C)
MAKE_SET(MatchEnd,int,I)
MAKE_SET(MaxAxes,int,I)
MAKE_SET(MinAxes,int,I)
MAKE_SET(Permute,int,I)
MAKE_SET(PreserveAxes,int,I)
MAKE_SET(Title,const char *,C)
MAKE_TEST(Digits)
MAKE_TEST(Domain)
MAKE_TEST(MatchEnd)
MAKE_TEST(MaxAxes)
MAKE_TEST(MinAxes)
MAKE_TEST(Permute)
MAKE_TEST(PreserveAxes)
MAKE_TEST(Title)

MAKE_GET(ActiveUnit,int)
MAKE_SET(ActiveUnit,int,I)
MAKE_TEST(ActiveUnit)

MAKE_GET(System,AstSystemType)
MAKE_SET_SYSTEM(System)
MAKE_TEST(System)
MAKE_CLEAR(System)

MAKE_GET(AlignSystem,AstSystemType)
MAKE_SET_SYSTEM(AlignSystem)
MAKE_TEST(AlignSystem)
MAKE_CLEAR(AlignSystem)

MAKE_GET(Epoch,double)
MAKE_SET(Epoch,double,D)
MAKE_TEST(Epoch)
MAKE_CLEAR(Epoch)

/* Clear, Get, Set and Test axis-dependent Frame attributes. */
MAKE_CLEAR_AXIS(Direction)
MAKE_CLEAR_AXIS(Format)
MAKE_CLEAR_AXIS(Label)
MAKE_CLEAR_AXIS(Symbol)
MAKE_CLEAR_AXIS(Unit)
MAKE_GET_AXIS(Direction,int)
MAKE_GET_AXIS(Format,const char *)
MAKE_GET_AXIS(Label,const char *)
MAKE_GET_AXIS(Symbol,const char *)
MAKE_GET_AXIS(Unit,const char *)
MAKE_SET_AXIS(Direction,int,I)
MAKE_SET_AXIS(Format,const char *,C)
MAKE_SET_AXIS(Label,const char *,C)
MAKE_SET_AXIS(Symbol,const char *,C)
MAKE_SET_AXIS(Unit,const char *,C)
MAKE_TEST_AXIS(Direction)
MAKE_TEST_AXIS(Format)
MAKE_TEST_AXIS(Label)
MAKE_TEST_AXIS(Symbol)
MAKE_TEST_AXIS(Unit)

MAKE_GET_AXIS(Bottom,double)
MAKE_SET_AXIS(Bottom,double,D)
MAKE_TEST_AXIS(Bottom)
MAKE_CLEAR_AXIS(Bottom)

MAKE_GET_AXIS(Top,double)
MAKE_SET_AXIS(Top,double,D)
MAKE_TEST_AXIS(Top)
MAKE_CLEAR_AXIS(Top)

/* Copy constructor. */
/* ----------------- */
static void Copy( const AstObject *objin, AstObject *objout ) {
/*
*  Name:
*     Copy

*  Purpose:
*     Copy constructor for Region objects.

*  Type:
*     Private function.

*  Synopsis:
*     void Copy( const AstObject *objin, AstObject *objout )

*  Description:
*     This function implements the copy constructor for Region objects.

*  Parameters:
*     objin
*        Pointer to the object to be copied.
*     objout
*        Pointer to the object being constructed.

*  Notes:
*     -  This constructor makes a deep copy.
*/

/* Local Variables: */
   AstRegion *in;                /* Pointer to input Region */
   AstRegion *out;               /* Pointer to output Region */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain pointers to the input and output Regions. */
   in = (AstRegion *) objin;
   out = (AstRegion *) objout;

/* For safety, first clear any references to the input memory from
   the output Region. */
   out->basemesh = NULL;
   out->frameset = NULL;
   out->points = NULL;
   out->unc = NULL;

/* Now copy each of the above structures. */
   out->frameset = astCopy( in->frameset );
   if( in->points ) out->points = astCopy( in->points );
   if( in->basemesh ) out->basemesh = astCopy( in->basemesh );
   if( in->unc ) out->unc = astCopy( in->unc );
}


/* Destructor. */
/* ----------- */
static void Delete( AstObject *obj ) {
/*
*  Name:
*     Delete

*  Purpose:
*     Destructor for Region objects.

*  Type:
*     Private function.

*  Synopsis:
*     void Delete( AstObject *obj )

*  Description:
*     This function implements the destructor for Region objects.

*  Parameters:
*     obj
*        Pointer to the object to be deleted.

*  Notes:
*     This function attempts to execute even if the global error status is
*     set.
*/

/* Local Variables: */
   AstRegion *this;                 /* Pointer to Region */

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) obj;

/* Annul all resources. */
   this->frameset = astAnnul( this->frameset );
   if( this->points ) this->points = astAnnul( this->points );
   if( this->basemesh ) this->basemesh = astAnnul( this->basemesh );
   if( this->unc ) this->unc = astAnnul( this->unc );
}

/* Dump function. */
/* -------------- */
static void Dump( AstObject *this_object, AstChannel *channel ) {
/*
*  Name:
*     Dump

*  Purpose:
*     Dump function for Region objects.

*  Type:
*     Private function.

*  Synopsis:
*     void Dump( AstObject *this, AstChannel *channel )

*  Description:
*     This function implements the Dump function which writes out data
*     for the Region class to an output Channel.

*  Parameters:
*     this
*        Pointer to the Region whose data are being written.
*     channel
*        Pointer to the Channel to which the data are being written.
*/

/* Local Constants: */
#define KEY_LEN 50               /* Maximum length of a keyword */
#define COM_LEN 50               /* Maximum length of a comment */

/* Local Variables: */
   AstFrame *fr;                 /* Pointer to the current Frame */
   AstMapping *smap;             /* Base->current Mapping */
   AstRegion *this;              /* Pointer to the Region structure */
   AstRegion *unc;               /* Pointer to the uncertainty Region */
   double dval;                  /* Floating point attribute value */
   int ival;                     /* Integer attribute value */
   int set;                      /* Attribute value set? */
   int unit;                     /* Base->current is unitmap? */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Region structure. */
   this = (AstRegion *) this_object;

/* Write out values representing the instance variables for the
   Region class.  Accompany these with appropriate comment strings,
   possibly depending on the values being written.*/

/* In the case of attributes, we first use the appropriate (private)
   Test...  member function to see if they are set. If so, we then use
   the (private) Get... function to obtain the value to be written
   out.

   For attributes which are not set, we use the astGet... method to
   obtain the value instead. This will supply a default value
   (possibly provided by a derived class which over-rides this method)
   which is more useful to a human reader as it corresponds to the
   actual default attribute value.  Since "set" will be zero, these
   values are for information only and will not be read back. */

/* Negated. */
/* -------- */
   set = TestNegated( this );
   ival = set ? GetNegated( this ) : astGetNegated( this );
   astWriteInt( channel, "Negate", (ival != 0), 0, ival,
                ival ? "Region negated" : "Region not negated" );

/* FillFactor */
/* ---------- */
   set = TestFillFactor( this );
   dval = set ? GetFillFactor( this ) : astGetFillFactor( this );
   astWriteDouble( channel, "Fill", set, 0, dval,"Region fill factor" );

/* MeshSize. */
/* --------- */
   set = TestMeshSize( this );
   ival = set ? GetMeshSize( this ) : astGetMeshSize( this );
   astWriteInt( channel, "MeshSz", set, 0, ival,
                "No. of points used to represent boundary" );

/* Closed. */
/* ------- */
   set = TestClosed( this );
   ival = set ? GetClosed( this ) : astGetClosed( this );
   astWriteInt( channel, "Closed", set, 0, ival,
                ival ? "Boundary is inside" : "Boundary is outside" );

/* FrameSet */
/* -------- */
/* We do not dump the FrameSet if the RegionFS attribute is zero. */
   if( astGetRegionFS( this ) ) {

/* If the vertices are the same in both base and current Frames (i.e. 
   if the Frames are connected by a UnitMap), then just dump the current 
   Frame. Otherwise, dump the whole FrameSet. */
      smap = RegMapping( this );
      if( ( unit = astIsAUnitMap( smap ) ) ){
         fr = astGetFrame( this->frameset, AST__CURRENT );
         astWriteObject( channel, "Frm", 1, 1, fr, "Coordinate system" );
         fr = astAnnul( fr );

      } else {
         astWriteObject( channel, "FrmSet", 1, 1, this->frameset, 
                         "Original & current coordinate systems" );
      }

/* Annul the Mapping pointers */
      smap = astAnnul( smap );
   }

/* Points */
/* ------ */
   if( this->points ) {
      astWriteObject( channel, "Points", 1, 1, this->points, 
                      "Points defining the shape" );
   }

/* Uncertainty */
/* ----------- */
/* Only dump the uncertinaty Region if required. */
   if( astDumpUnc( this ) ) {
      unc = astGetUnc( this, AST__BASE );
      astWriteObject( channel, "Unc", 1, 1, unc, 
                      "Region defining positional uncertainties." );
      unc = astAnnul( unc );
   }

/* Undefine macros local to this function. */
#undef KEY_LEN
}


/* Standard class functions. */
/* ========================= */
/* Implement the astIsARegion and astCheckRegion functions using
   the macros defined for this purpose in the "object.h" header
   file. */
astMAKE_ISA(Region,Frame,check,&class_init)
astMAKE_CHECK(Region)

AstRegion *astInitRegion_( void *mem, size_t size, int init,
                           AstRegionVtab *vtab, const char *name,
                           AstFrame *frame, AstPointSet *pset,
                           AstRegion *unc ){
/*
*+
*  Name:
*     astInitRegion

*  Purpose:
*     Initialise a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstRegion *astInitRegion( void *mem, size_t size, int init,
*                               AstRegionVtab *vtab, const char *name,
*                               AstFrame *frame, AstpointSet *pset,
*                               AstRegion *unc )

*  Class Membership:
*     Region initialiser.

*  Description:
*     This function is provided for use by class implementations to
*     initialise a new Region object. It allocates memory (if
*     necessary) to accommodate the Region plus any additional data
*     associated with the derived class.  It then initialises a
*     Region structure at the start of this memory. If the "init"
*     flag is set, it also initialises the contents of a virtual
*     function table for a Region at the start of the memory passed
*     via the "vtab" parameter.

*  Parameters:
*     mem
*        A pointer to the memory in which the Region is to be
*        created. This must be of sufficient size to accommodate the
*        Region data (sizeof(Region)) plus any data used by the
*        derived class. If a value of NULL is given, this function
*        will allocate the memory itself using the "size" parameter to
*        determine its size.
*     size
*        The amount of memory used by the Region (plus derived class
*        data).  This will be used to allocate memory if a value of
*        NULL is given for the "mem" parameter. This value is also
*        stored in the Region structure, so a valid value must be
*        supplied even if not required for allocating memory.
*     init
*        A logical flag indicating if the Region's virtual function
*        table is to be initialised. If this value is non-zero, the
*        virtual function table will be initialised by this function.
*     vtab
*        Pointer to the start of the virtual function table to be
*        associated with the new Region.
*     name
*        Pointer to a constant null-terminated character string which
*        contains the name of the class to which the new object
*        belongs (it is this pointer value that will subsequently be
*        returned by the astGetClass method).
*     frame
*        Pointer to the encapsulated Frame. A deep copy of this Frame is 
*        taken. This means that subsequent changes to the supplied Frame
*        will have no effect on the new Region.
*     pset
*        A PointSet holding the points which define the Region. These
*        positions should refer to the given Frame. May be NULL.
*     unc
*        A pointer to a Region which specifies the uncertainty in the
*        supplied positions (all points on the boundary of the new Region
*        being initialised are assumed to have the same uncertainty). A NULL 
*        pointer can be supplied, in which case default uncertainties equal to 
*        1.0E-6 of the dimensions of the new Region's bounding box are used. 
*        If an uncertainty Region is supplied, it must be either a Box, a 
*        Circle or an Ellipse, and its encapsulated Frame must be related
*        to the Frame supplied for parameter "frame" (i.e. astConvert
*        should be able to find a Mapping between them). Two positions 
*        the "frame" Frame are considered to be co-incident if their 
*        uncertainty Regions overlap. The centre of the supplied
*        uncertainty Region is immaterial since it will be re-centred on the 
*        point being tested before use. A deep copy is taken of the supplied 
*        Region.

*  Returned Value:
*     A pointer to the new Region.

*  Notes:
*     - A null pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   AstFrame *f0;                  /* Frame to use */
   AstRegion *new;                /* Pointer to new Region */
   int nax;                       /* No. of axes in supplied Frame */
   int ncoord;                    /* Coords per point */

/* Check the global status. */
   if ( !astOK ) return NULL;

/* If necessary, initialise the virtual function table. */
   if ( init ) astInitRegionVtab( vtab, name );

/* Note the number of axes in the supplied Frame. */
   nax = astGetNaxes( frame );

/* Check the pointset if supplied. */
   if( pset ) {

/* Note the number of axes per point in the supplied PointSet */
      ncoord = astGetNcoord( pset );

/* If OK, check that the number of coordinates per point matches the number
   of axes in the Frame. Report an error if these numbers do not match. */
      if ( astOK && ( ncoord != nax ) ) {
         astError( AST__NCPIN, "astInitRegion(%s): Bad number of coordinate "
                   "values per point (%d).", name, ncoord );
         astError( AST__NCPIN, "The %s given requires %d coordinate value(s) "
                   "for each point.", astGetClass( frame ), nax );
      }
   }

/* Initialise a Frame structure (the parent class) as the first
   component within the Region structure, allocating memory if
   necessary. Give this Frame zero axes as the Frame information will be
   specified by the encapsulated FrameSet. */
   new = (AstRegion *) astInitFrame( mem, size, 0, (AstFrameVtab *) vtab, 
                                     name, 0 );
   if ( astOK ) {

/* Initialise the Region data. */
/* ----------------------------- */
      new->meshsize = -INT_MAX;
      new->basemesh = NULL;
      new->negated = -INT_MAX;
      new->closed = -INT_MAX;
      new->regionfs = -INT_MAX;
      new->fillfactor = AST__BAD;

/* If the supplied Frame is a Region, gets its encapsulated Frame. If a 
   FrameSet was supplied, use its current Frame, otherwise use the
   supplied Frame. */
      if( astIsARegion( frame ) ) {
         f0 = astGetFrame( ((AstRegion *) frame)->frameset, AST__CURRENT );

      } else if( astIsAFrameSet( frame ) ) {
         f0 = astGetFrame( (AstFrameSet *) frame, AST__CURRENT );

      } else { 
         f0 = astClone( frame );
      }    

/* Form a FrameSet consisting of two copies of the supplied Frame
   connected together by a UnitMap, and store in the Region structure. */
      astSetRegFS( new, f0 );

/* Store a clone of the supplied PointSet pointer. */
      new->points = pset ? astClone( pset ) : NULL;

/* Store any uncertainty Region. */
      new->unc = NULL;
      astSetUnc( new, unc );

/* If an error occurred, clean up by deleting the new object. */
      if ( !astOK ) new = astDelete( new );
   }

/* Return a pointer to the new object. */
   return new;
}

AstRegion *astLoadRegion_( void *mem, size_t size,
                           AstRegionVtab *vtab, const char *name,
                           AstChannel *channel ) {
/*
*+
*  Name:
*     astLoadRegion

*  Purpose:
*     Load a Region.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "region.h"
*     AstRegion *astLoadRegion( void *mem, size_t size,
*                               AstRegionVtab *vtab, const char *name,
*                               AstChannel *channel )

*  Class Membership:
*     Region loader.

*  Description:
*     This function is provided to load a new Region using data read
*     from a Channel. It first loads the data used by the parent class
*     (which allocates memory if necessary) and then initialises a
*     Region structure in this memory, using data read from the
*     input Channel.

*  Parameters:
*     mem
*        A pointer to the memory into which the Region is to be
*        loaded.  This must be of sufficient size to accommodate the
*        Region data (sizeof(Region)) plus any data used by
*        derived classes. If a value of NULL is given, this function
*        will allocate the memory itself using the "size" parameter to
*        determine its size.
*     size
*        The amount of memory used by the Region (plus derived class
*        data).  This will be used to allocate memory if a value of
*        NULL is given for the "mem" parameter. This value is also
*        stored in the Region structure, so a valid value must be
*        supplied even if not required for allocating memory.
*
*        If the "vtab" parameter is NULL, the "size" value is ignored
*        and sizeof(AstRegion) is used instead.
*     vtab
*        Pointer to the start of the virtual function table to be
*        associated with the new Region. If this is NULL, a pointer
*        to the (static) virtual function table for the Region class
*        is used instead.
*     name
*        Pointer to a constant null-terminated character string which
*        contains the name of the class to which the new object
*        belongs (it is this pointer value that will subsequently be
*        returned by the astGetClass method).
*
*        If the "vtab" parameter is NULL, the "name" value is ignored
*        and a pointer to the string "Region" is used instead.

*  Returned Value:
*     A pointer to the new Region.

*  Notes:
*     - A null pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Constants: */
#define KEY_LEN 50               /* Maximum length of a keyword */

/* Local Variables: */
   AstFrame *f1;                  /* Base Frame for encapsulated FrameSet */
   AstRegion *new;                /* Pointer to the new Region */
   AstRegion *unc;                /* Pointer to the uncertainty Region */
   int nax;                       /* No. of axes in Frame */
   int naxpt;                     /* No. of axes in per point */

/* Initialise. */
   new = NULL;

/* Check the global error status. */
   if ( !astOK ) return new;

/* If a NULL virtual function table has been supplied, then this is
   the first loader to be invoked for this Region. In this case the
   Region belongs to this class, so supply appropriate values to be
   passed to the parent class loader (and its parent, etc.). */
   if ( !vtab ) {
      size = sizeof( AstRegion );
      vtab = &class_vtab;
      name = "Region";

/* If required, initialise the virtual function table for this class. */
      if ( !class_init ) {
         astInitRegionVtab( vtab, name );
         class_init = 1;
      }
   }

/* Invoke the parent class loader to load data for all the ancestral
   classes of the current one, returning a pointer to the resulting
   partly-built Region. */
   new = astLoadFrame( mem, size, (AstFrameVtab *) vtab, name,
                       channel );

   if ( astOK ) {

/* Read input data. */
/* ================ */
/* Request the input Channel to read all the input data appropriate to
   this class into the internal "values list". */
      astReadClassData( channel, "Region" );

/* Now read each individual data item from this list and use it to
   initialise the appropriate instance variable(s) for this class. */

/* In the case of attributes, we first read the "raw" input value,
   supplying the "unset" value as the default. If a "set" value is
   obtained, we then use the appropriate (private) Set... member
   function to validate and set the value properly. */

/* Negated */
/* ------- */
      new->negated = astReadInt( channel, "negate", -INT_MAX );
      if ( TestNegated( new ) ) SetNegated( new, new->negated );

/* FillFactor */
/* ---------- */
      new->fillfactor = astReadDouble( channel, "fill", AST__BAD );
      if ( TestFillFactor( new ) ) SetFillFactor( new, new->fillfactor );

/* MeshSize */
/* -------- */
      new->meshsize = astReadInt( channel, "meshsz", -INT_MAX );
      if ( TestMeshSize( new ) ) SetMeshSize( new, new->meshsize );

/* Closed */
/* ------ */
      new->closed = astReadInt( channel, "closed", -INT_MAX );
      if ( TestClosed( new ) ) SetClosed( new, new->closed );

/* Points */
/* ------ */
      new->points = astReadObject( channel, "points", NULL );

/* If some points were found, ensure that they are in a PointSet and get
   the number of axis values per point. */
      naxpt = 0;
      if( new->points ){
         if( astIsAPointSet( new->points) ) {
            naxpt = astGetNcoord( new->points );
         } else {
            astError( AST__REGIN, "astLoadRegion(%s): Corrupt %s specifies points "
                      "using a %s (should be a PointSet).", astGetClass( new ),
                      astGetClass( new ), astGetClass( new->points ) );
         }
      }

/* FrameSet */
/* -------- */
/* First see if the dump contains a single Frame. If so, create a
   FrameSet from it and a copy of itself, using a UnitMap to connect the
   two. */
      new->frameset = NULL;
      new->regionfs = 1;
      f1 = astReadObject( channel, "frm", NULL );
      if( f1 ) {
         nax = astGetNaxes( f1 );
         astSetRegFS( new, f1 );
         f1 = astAnnul( f1 );

/* If no Frame was found in the dump, look for a FrameSet. */
      } else {
         new->frameset = astReadObject( channel, "frmset", NULL );
         if( new->frameset ) {
            nax = astGetNaxes( new->frameset );
         } else {
            nax = 0;
         }
      }   

/* If neither a Frame nor a FrameSet was found, create a default FrameSet
   and set the RegionFS attribute false, to indicate that the FrameSet
   should not be used. */
      if( !new->frameset ){
         nax = naxpt ? naxpt : 1;
         f1 = astFrame( nax, "" );
         new->frameset = astFrameSet( f1, "" );
         f1 = astAnnul( f1 );
         new->regionfs = 0;
      }

/* Report an error if the number of axis values per point in the pointset is 
   incorrect. */
      if ( astOK && new->points && ( naxpt != nax ) ) {
         astError( AST__REGIN, "astLoadRegion(%s): Corrupt %s contains "
                   " incorrect number of coordinate values per point (%d).", 
                   astGetClass( new ), astGetClass( new ), naxpt );
         astError( AST__REGIN, "The %s requires %d coordinate value(s) "
                   "for each point.", astGetClass( new ), nax );
      }
   

/* Uncertainty */
/* ----------- */
      unc = astReadObject( channel, "unc", NULL );
      if( unc ) {

/* If the uncertainty Region has a zero value for its RegionFS attribute,
   it will currently contain a dummy FrameSet rather than the correct
   FrameSet. The correct FrameSet has copies of the base Frame of the new
   Region as both its current and base Frames, and these are connected by
   a UnitMap (this is equivalent to a FrameSet containing a single Frame). */
         if( !astGetRegionFS( unc ) ) {
            f1 = astGetFrame( new->frameset, AST__BASE );
            astSetRegFS( unc, f1 );
            f1 = astAnnul( f1 );
         }

/* Store the corrected uncertainty Region in the new Region. */
         new->unc = NULL;
         astSetUnc( new, unc );
     }

/* Initialise other fields which are used as caches for values derived
   from the attributes set above. */
      new->basemesh = NULL;

/* If an error occurred, clean up by deleting the new Region. */
      if ( !astOK ) new = astDelete( new );
   }

/* Return the new Region pointer. */
   return new;

/* Undefine macros local to this function. */
#undef KEY_LEN
}

/* Virtual function interfaces. */
/* ============================ */
/* These provide the external interface to the virtual functions defined by
   this class. Each simply checks the global error status and then locates and
   executes the appropriate member function, using the function pointer stored
   in the object's virtual function table (this pointer is located using the
   astMEMBER macro defined in "object.h").

   Note that the member function may not be the one defined here, as it may
   have been over-ridden by a derived class. However, it should still have the
   same interface. */

void astNegate_( AstRegion *this ){
   if ( !astOK ) return;
   (**astMEMBER(this,Region,Negate))( this );
}
AstFrame *astGetRegionFrame_( AstRegion *this ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,GetRegionFrame))( this );
}
AstRegion *astMapRegion_( AstRegion *this, AstMapping *map, AstFrame *frame ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,MapRegion))( this, map, frame );
}
int astOverlap_( AstRegion *this, AstRegion *that ){
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Region,Overlap))( this, that );
}
int astOverlapX_( AstRegion *that, AstRegion *this ){
   if ( !astOK ) return 0;
   return (**astMEMBER(that,Region,OverlapX))( that, this );
}
AstFrame *astRegFrame_( AstRegion *this ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,RegFrame))( this );
}
AstPointSet *astRegTransform_( AstRegion *this, AstPointSet *in,
                               int forward, AstPointSet *out,
                               AstFrame **frm ) {
   if( frm ) *frm = NULL;
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,RegTransform))( this, in, forward, out, frm );
}
int astRegPins_( AstRegion *this, AstPointSet *pset, AstRegion *unc, int **mask ){
   if( mask ) *mask = NULL;
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Region,RegPins))( this, pset, unc, mask );
}
int astDumpUnc_( AstRegion *this ){
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Region,DumpUnc))( this );
}
int astGetBounded_( AstRegion *this ){
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Region,GetBounded))( this );
}
int astTestUnc_( AstRegion *this ){
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Region,TestUnc))( this );
}
void astClearUnc_( AstRegion *this ){
   if ( !astOK ) return;
   (**astMEMBER(this,Region,ClearUnc))( this );
}
void astRegBaseBox_( AstRegion *this, double *lbnd, double *ubnd ){
   if ( !astOK ) return;
   (**astMEMBER(this,Region,RegBaseBox))( this, lbnd, ubnd );
}
void astRegCurBox_( AstRegion *this, double *lbnd, double *ubnd ){
   if ( !astOK ) return;
   (**astMEMBER(this,Region,RegCurBox))( this, lbnd, ubnd );
}
void astRegOverlay_( AstRegion *this, AstRegion *that ){
   if ( !astOK ) return;
   (**astMEMBER(this,Region,RegOverlay))( this, that );
}
AstPointSet *astRegMesh_( AstRegion *this ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,RegMesh))( this );
}
double *astRegCentre_( AstRegion *this, double *cen, double **ptr, int index,
                       int ifrm ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,RegCentre))( this, cen, ptr, index, ifrm );
}
AstRegion *astGetUnc_( AstRegion *this, int ifrm ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,GetUnc))( this, ifrm );
}
AstRegion *astGetDefUnc_( AstRegion *this ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,GetDefUnc))( this );
}
void astSetUnc_( AstRegion *this, AstRegion *unc ){
   if ( !astOK ) return;
   (**astMEMBER(this,Region,SetUnc))( this, unc );
}
void astSetRegFS_( AstRegion *this, AstFrame *frm ){
   if ( !astOK ) return;
   (**astMEMBER(this,Region,SetRegFS))( this, frm );
}
AstPointSet *astRegBaseMesh_( AstRegion *this ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,RegBaseMesh))( this );
}
AstPointSet *astBndBaseMesh_( AstRegion *this, double *lbnd, double *ubnd ){
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Region,BndBaseMesh))( this, lbnd, ubnd );
}

#define MAKE_MASK_(X,Xtype) \
int astMask##X##_( AstRegion *this, AstMapping *map, int inside, int ndim, \
                   const int lbnd[], const int ubnd[], Xtype in[], \
                   Xtype val ) { \
   if ( !astOK ) return 0; \
   return (**astMEMBER(this,Region,Mask##X))( this, map, inside, ndim, lbnd, \
                                              ubnd, in, val ); \
}
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_MASK_(LD,long double)
#endif
MAKE_MASK_(D,double)
MAKE_MASK_(F,float)
MAKE_MASK_(L,long int)
MAKE_MASK_(UL,unsigned long int)
MAKE_MASK_(I,int)
MAKE_MASK_(UI,unsigned int)
MAKE_MASK_(S,short int)
MAKE_MASK_(US,unsigned short int)
MAKE_MASK_(B,signed char)
MAKE_MASK_(UB,unsigned char)
#undef MAKE_MASK_

/* Special public interface functions. */
/* =================================== */
/* These provide the public interface to certain special functions
   whose public interface cannot be handled using macros (such as
   astINVOKE) alone. In general, they are named after the
   corresponding protected version of the function, but with "Id"
   appended to the name. */

/* Public Interface Function Prototypes. */
/* ------------------------------------- */
/* The following functions have public prototypes only (i.e. no
   protected prototypes), so we must provide local prototypes for use
   within this module. */

/* Special interface function implementations. */
/* ------------------------------------------- */


AstRegion *astMapRegionId_( AstRegion *this, AstMapping *map, AstFrame *frame ) {
/*
*++
*  Name:
c     astMapRegion
f     AST_MAPREGION

*  Purpose:
*     Transform a Region into a new Frame using a given Mapping.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "region.h"
c     AstRegion *astMapRegion( AstRegion *this, AstMapping *map,
c                              AstFrame *frame ) 
f     RESULT = AST_MAPREGION( THIS, MAP, FRAME, STATUS )

*  Class Membership:
*     Region method.

*  Description:
*     This function returns a pointer to a new Region which corresponds to
*     supplied Region described by some other specified coordinate system. A
*     Mapping is supplied which transforms positions between the old and new
*     coordinate systems. The new Region may not be of the same class as
*     the original region.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Region.
c     map
f     MAP = INTEGER (Given)
*        Pointer to a Mapping which transforms positions from the
*        coordinate system represented by the supplied Region to the 
*        coordinate system specified by 
c        "frame".
f        FRAME.
*        The supplied Mapping should define both forward and inverse 
*        transformations, and these transformations should form a genuine
*        inverse pair. That is, transforming a position using the forward
*        transformation and then using the inverse transformation should
*        produce the original input position. Some Mapping classes (such
*        as PermMap, MathMap, SphMap) can result in Mappings for which this 
*        is not true.
c     frame
f     FRAME = INTEGER (Given)
*        Pointer to a Frame describing the coordinate system in which 
*        the new Region is required.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Returned Value:
c     astMapRegion()
f     AST_MAPREGION = INTEGER
*        A pointer to a new Region. This Region will represent the area
*        within the coordinate system specified by
c        "frame"
f        FRAME
*        which corresponds to the supplied Region.

*  Notes:
*     - The uncertainty associated with the supplied Region is modified 
*     using the supplied Mapping.
*     - A null Object pointer (AST__NULL) will be returned if this
c     function is invoked with the AST error status set, or if it
f     function is invoked with STATUS set to an error value, or if it
*     should fail for any reason.
*--

*  Implementation Notes:
*     - The only difference between this public interface and the protected 
*     astMapRegion interface is that this implementation additionally
*     simplifies the returned Region. The protected implementation does
*     not do this since doing so can lead to infinite recursion because
*     it is sometimes necessary for Simplify to call astMapRegion.

*/

/* Local Variables: */
   AstRegion *new;                /* Pointer to new Region */
   AstRegion *result;             /* Pointer value to return */

/* Initialise. */
   result = NULL;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Invoke the protected astMapRegion function. */
   new = astMapRegion( this, map, frame );

/* Simplify the resulting Region. */
   result = astSimplify( new ); 

/* Free resources. */
   new = astAnnul( new );

/* If not OK, annul the returned pointer. */
   if( !astOK ) result = astAnnul( result );

/* Return the result. */
   return result;
}






