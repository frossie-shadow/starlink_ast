#if !defined( SKYFRAME_INCLUDED ) /* Include this file only once */
#define SKYFRAME_INCLUDED
/*
*+
*  Name:
*     skyframe.h

*  Type:
*     C include file.

*  Purpose:
*     Define the interface to the SkyFrame class.

*  Invocation:
*     #include "skyframe.h"

*  Description:
*     This include file defines the interface to the SkyFrame class
*     and provides the type definitions, function prototypes and
*     macros, etc. needed to use this class.

*  Inheritance:
*     The SkyFrame class inherits from the Frame class.

*  Attributes Over-Ridden:
*     Direction(axis) (integer)
*        Specifies in which direction an axis should be plotted
*        (e.g. in a graph) to display coordinate values for each
*        SkyFrame axis. The SkyFrame class re-defines the default so
*        that certain axes (e.g. Right Ascension) are plotted in
*        reverse when appropriate.
*     Domain (string)
*        A string which may be used to identify a SkyFrame and used as
*        an additional key when matching a target SkyFrame with a
*        template.  The SkyFrame class re-defines the default value to
*        "SKY".
*     Format(axis) (string)
*        Specifies the format to be used to display coordinate values
*        for each SkyFrame axis (i.e. to convert them from binary
*        radians to character form). The SkyFrame class re-defines the
*        syntax of this string (e.g. "dms" or "zhms.3"), and its
*        default, to allow the formatting of sexagessimal values in a
*        manner appropriate to the sky coordinate system being
*        represented. The syntax of SkyFrame Format strings is
*        identical to that defined by the SkyAxis class.
*     Label(axis) (string)
*        Specifies the label to be attached to each SkyFrame axis when
*        it is represented in (e.g.) a graph. The SkyFrame class
*        re-defines the default values (e.g. to "Right Ascension") to
*        be appropriate to the sky coordinate system being
*        represented.
*     MaxAxes (integer)
*        Specifies the maximum number of axes in a target Frame that
*        can be matched when using the SkyFrame as a template. The
*        SkyFrame class constrains this to be 2.
*     MinAxes (integer)
*        Specifies the minimum number of axes in a target Frame that
*        can be matched when using the SkyFrame as a template. The
*        SkyFrame class constrains this to be 2.
*     Permute (integer)
*        A boolean value which specifies whether the axis order of a
*        target Frame may be permuted in order to obtain a match with
*        a SkyFrame template. If this value is set to zero in the
*        template SkyFrame, it will only match a target if it can do
*        so without changing the order of its axes. The SkyFrame class
*        implements this restriction (it is effectively ignored by the
*        Frame class which does not distinguish axes from each other,
*        and will therefore match any target Frame axis order).
*     Symbol(axis) (string)
*        Specifies the symbol to be used to represent coordinate
*        values for each SkyFrame axis in "short form", such as in
*        algebraic expressions where a full description of the axis
*        would be inappropriate. The SkyFrame class re-defines the
*        default values (e.g. to "RA" and "Dec") to be appropriate to
*        the sky coordinate system being represented.
*     System (string)
*        This attribute takes one of the following values to identify the 
*        sky coordinate system that the SkyFrame represents:
*
*        "FK4"
*           The old FK4 (barycentric) equatorial coordinate
*           system.
*        "FK4-NO-E" or "FK4_NO_E"
*           The old FK4 (barycentric) equatorial system but without
*           the "E-terms of aberration" (e.g. some radio
*           catalogues).
*        "FK5" or "EQUATORIAL"
*           The modern FK5 (barycentric) equatorial coordinate
*           system.
*        "ICRS" 
*           The International Celestial Reference System, realised
*           through the Hipparcos catalogue.
*        "GAPPT", "APPARENT" or "GEOCENTRIC"
*           The geocentric apparent equatorial coordinate system,
*           which gives the apparent positions of objects relative to
*           the true plane of the Earth's equator and the equinox
*           (the coordinate origin) at a time specified by the
*           qualifying Epoch value.
*        "ECLIPTIC"
*           Ecliptic coordinates (IAU 1980), referred to the ecliptic
*           and mean equinox as specified by the qualifying Equinox
*           value.
*        "HELIOECLIPTIC"
*           Ecliptic coordinates (IAU 1980), referred to the ecliptic
*           and mean equinox of J2000.0, in which an offset is added to
*           the longitude value which results in the centre of the Sun
*           being at zero longitude at the date given by the Epoch attribute.
*        "GALACTIC"
*           Galactic coordinates (IAU 1958).
*        "SUPERGALACTIC"
*           De Vaucouleurs Supergalactic coordinates.
*        "UNKNOWN"
*           An unknown spherical coordinate system.
*     Title (string)
*        Specifies a string to be used as a title on (e.g.) graphs to
*        describe the coordinate system which the SkyFrame
*        represents. The SkyFrame class re-defines the default to
*        describe the sky coordinate system being represented
*        (e.g. "FK5 Equatorial Coordinates, Mean Equinox J2000.0").
*     Unit(axis) (string)
*        Describes the units used to represent coordinate values on
*        each SkyFrame axis. The SkyFrame class re-defines the default
*        to describe the character string returned by the astFormat
*        method when formatting sexagessimal values
*        (e.g. "hh:mm:ss.sss").

*  New Attributes Defined:
*     AsTime(axis) (integer)
*        A boolean value that specifies whether each axis of a
*        SkyFrame should have its values formatted as times (e.g. in
*        hours, minutes and seconds) rather than as angles (in
*        degrees). This value affects the default value of the Format
*        and Unit attributes for each axis and the operation of the
*        astFormat method. The default value is chosen to be
*        appropriate to the sky coordinate system being represented.
*     Equinox (double)
*        This value is used to qualify sky coordinate systems that are
*        notionally based on the ecliptic (the plane of the Earth's
*        orbit around the Sun) and/or the Earth's equator.
*     LatAxis (integer)
*        A read-only attribute giving the index of the latitude axis, 
*        taking account of any current axis permutation.
*     LonAxis (integer)
*        A read-only attribute giving the index of the longitude axis, 
*        taking account of any current axis permutation.
*     NegLon (integer)
*        A boolean value that controls how a longitude value is
*        normalized by astNorm. If non-zero, then longitude values are
*        normalized into the range [-pi,+pi]. Otherwise (the default), 
*        they are normalized into the range [0,2.pi].
*     Projection (string)
*        This attribute contains a human-readable description of the
*        type of sky projection used when a SkyFrame is attached to a
*        2-dimensional object such as an image or plotting surface
*        (for example, "tangent-plane projection"). It does not affect
*        the behaviour of the SkyFrame, but serves simply to inform.
*        If set to a non-blank value, it may appear as part of the
*        default value of the SkyFrame's Title attribute. The default
*        value is an empty string.

*  Methods Over-Ridden:
*     Public:
*        astDistance
*           Calculate the distance between two points.
*        astFormat
*           Format a coordinate value for a SkyFrame axis.
*        astNorm
*           Normalise a set of SkyFrame coordinates.
*        astOffset
*           Calculate an offset along a geodesic curve.
*        astUnformat
*           Read a formatted coordinate value for a SkyFrame axis.

*     Protected:
*        astGap
*           Find a "nice" gap for tabulating SkyFrame axis values.
*        astGetDirection
*           Obtain the value of the Direction attribute for a SkyFrame axis.
*        astGetDomain
*           Obtain a pointer to the Domain string for a SkyFrame axis.
*        astGetFormat
*           Obtain a pointer to the Format string for a SkyFrame axis.
*        astGetLabel
*           Obtain a pointer to the Label string for a SkyFrame axis.
*        astGetSymbol
*           Obtain a pointer to the Symbol string for a SkyFrame axis.
*        astGetTitle
*           Obtain a pointer to the Title string for a SkyFrame axis.
*        astGetUnit
*           Obtain a pointer to the Unit string for a SkyFrame axis.
*        astMatch
*           Determine if conversion is possible between two coordinate systems.
*        astOverlay
*           Overlay the attributes of a template SkyFrame on to another Frame.
*        astSetAttrib
*           Set an attribute value for a SkyFrame.
*        astSetMaxAxes
*           Set a value for the MaxAxes attribute of a SkyFrame.
*        astSetMinAxes
*           Set a value for the MinAxes attribute of a SkyFrame.
*        astSubFrame
*           Select axes from a SkyFrame and convert to the new coordinate
*           system.

*  New Methods Defined:
*     Public
*        None.

*     Protected:
*        astClearAsTime
*           Clear the value of the AsTime attribute for a SkyFrame axis.
*        astClearEquinox
*           Clear the value of the Equinox attribute for a SkyFrame.
*        astClearNegLon
*           Clear the value of the NegLon attribute for a SkyFrame.
*        astClearProjection
*           Clear the value of the Projection attribute for a SkyFrame.
*        astGetAsTime
*           Obtain the value of the AsTime attribute for a SkyFrame axis.
*        astGetEquinox
*           Obtain the value of the Equinox attribute for a SkyFrame.
*        astGetNegLon
*           Obtain the value of the NegLon attribute for a SkyFrame.
*        astGetProjection
*           Obtain the value of the Projection attribute for a SkyFrame.
*        astSetAsTime
*           Set a value for the AsTime attribute of a SkyFrame axis.
*        astSetEquinox
*           Set a value for the Equinox attribute of a SkyFrame.
*        astSetNegLon
*           Set a value for the NegLon attribute of a SkyFrame.
*        astSetProjection
*           Set a value for the Projection attribute of a SkyFrame.
*        astTestAsTime
*           Test if a value has been set for the AsTime attribute of a SkyFrame
*           axis.
*        astTestEquinox
*           Test if a value has been set for the Equinox attribute of a
*           SkyFrame.
*        astTestNegLon
*           Test if a value has been set for the NegLon attribute of a
*           SkyFrame.
*        astTestProjection
*           Test if a value has been set for the Projection attribute of a
*           SkyFrame.

*  Other Class Functions:
*     Public:
*        astIsASkyFrame
*           Test class membership.
*        astSkyFrame
*           Create a SkyFrame.

*     Protected:
*        astCheckSkyFrame
*           Validate class membership.
*        astInitSkyFrame
*           Initialise a SkyFrame.
*        astInitSkyFrameVtab
*           Initialise the virtual function table for the SkyFrame class.
*        astLoadSkyFrame
*           Load a SkyFrame.

*  Macros:
*     None.

*  Type Definitions:
*     Public:
*        AstSkyFrame
*           SkyFrame object type.

*     Protected:
*        AstSkyFrameVtab
*           SkyFrame virtual function table type.

*  Feature Test Macros:
*     astCLASS
*        If the astCLASS macro is undefined, only public symbols are
*        made available, otherwise protected symbols (for use in other
*        class implementations) are defined. This macro also affects
*        the reporting of error context information, which is only
*        provided for external calls to the AST library.

*  Copyright:
*     <COPYRIGHT_STATEMENT>

*  Authors:
*     RFWS: R.F. Warren-Smith (Starlink)
*     DSB: David S. Berry (Starlink)

*  History:
*     4-MAR-1996 (RFWS):
*        Original version.
*     24-MAY-1996 (RFWS):
*        Tidied up, etc.
*     24-SEP-1996 (RFWS):
*        Added I/O facilities.
*     16-JUL-1997 (RFWS):
*        Added Projection attribute.
*     26-FEB-1998 (RFWS):
*        Over-ride the astUnformat method.
*     3-APR-2001 (DSB):
*        Added "Unknown" option for the System attribute. Added read-only
*        attributes LatAxis and LonAxis.
*     10-OCT-2002 (DSB):
*        Moved definitions of macros for SkyFrame system values into
*        this file from skyframe.c.
*     15-NOV-2002 (DSB):
*        Move the System attribute from this class to the parent (Frame)
*        class.
*     8-JAN-2003 (DSB):
*        Added protected astInitSkyFrameVtab method.
*-
*/

/* Include files. */
/* ============== */
/* Interface definitions. */
/* ---------------------- */
#include "object.h"              /* Base Object class */
#include "frame.h"               /* Parent Frame class */

/* Macros. */
/* ======= */

#if defined(astCLASS)            /* Protected */

/* Values used to represent different System attribute values. */
#define AST__FK4           1
#define AST__FK4_NO_E      2
#define AST__FK5           3
#define AST__GAPPT         4
#define AST__ECLIPTIC      5
#define AST__GALACTIC      6
#define AST__SUPERGALACTIC 7
#define AST__ICRS          8
#define AST__HELIOECLIPTIC 9
#define AST__UNKNOWN       10

#endif

/* Type Definitions. */
/* ================= */

/* SkyFrame structure. */
/* ------------------- */
/* This structure contains all information that is unique to each object in
   the class (e.g. its instance variables). */
typedef struct AstSkyFrame {

/* Attributes inherited from the parent class. */
   AstFrame frame;               /* Parent class structure */

/* Attributes specific to objects in this class. */
   char *projection;             /* Description of sky projection */
   double equinox;               /* Modified Julian Date of mean equinox */
   int neglon;                   /* Display negative longitude values? */
} AstSkyFrame;

/* Virtual function table. */
/* ----------------------- */
/* This table contains all information that is the same for all objects in the
   class (e.g. pointers to its virtual functions). */
#if defined(astCLASS)            /* Protected */
typedef struct AstSkyFrameVtab {

/* Properties (e.g. methods) inherited from the parent class. */
   AstFrameVtab frame_vtab;      /* Parent class virtual function table */

/* Unique flag value to determine class membership. */
   int *check;                   /* Check value */

/* Properties (e.g. methods) specific to this class. */
   const char *(* GetProjection)( AstSkyFrame * );
   double (* GetEquinox)( AstSkyFrame * );
   int (* GetNegLon)( AstSkyFrame * );
   int (* GetAsTime)( AstSkyFrame *, int );
   int (* GetLatAxis)( AstSkyFrame * );
   int (* GetLonAxis)( AstSkyFrame * );
   int (* TestAsTime)( AstSkyFrame *, int );
   int (* TestEquinox)( AstSkyFrame * );
   int (* TestNegLon)( AstSkyFrame * );
   int (* TestProjection)( AstSkyFrame * );
   void (* ClearAsTime)( AstSkyFrame *, int );
   void (* ClearEquinox)( AstSkyFrame * );
   void (* ClearNegLon)( AstSkyFrame * );
   void (* ClearProjection)( AstSkyFrame * );
   void (* SetAsTime)( AstSkyFrame *, int, int );
   void (* SetEquinox)( AstSkyFrame *, double );
   void (* SetNegLon)( AstSkyFrame *, int );
   void (* SetProjection)( AstSkyFrame *, const char * );
} AstSkyFrameVtab;
#endif

/* Function prototypes. */
/* ==================== */
/* Prototypes for standard class functions. */
/* ---------------------------------------- */
astPROTO_CHECK(SkyFrame)         /* Check class membership */
astPROTO_ISA(SkyFrame)           /* Test class membership */

/* Constructor. */
#if defined(astCLASS)            /* Protected */
AstSkyFrame *astSkyFrame_( const char *, ... );
#else
AstSkyFrame *astSkyFrameId_( const char *, ... );
#endif

#if defined(astCLASS)            /* Protected */

/* Initialiser. */
AstSkyFrame *astInitSkyFrame_( void *, size_t, int, AstSkyFrameVtab *,
                               const char * );

/* Vtab initialiser. */
void astInitSkyFrameVtab_( AstSkyFrameVtab *, const char * );

/* Loader. */
AstSkyFrame *astLoadSkyFrame_( void *, size_t, AstSkyFrameVtab *,
                               const char *, AstChannel *channel );
#endif

/* Prototypes for member functions. */
/* -------------------------------- */
#if defined(astCLASS)            /* Protected */
const char *astGetProjection_( AstSkyFrame * );
double astGetEquinox_( AstSkyFrame * );
int astGetNegLon_( AstSkyFrame * );
int astGetAsTime_( AstSkyFrame *, int );
int astGetLatAxis_( AstSkyFrame * );
int astGetLonAxis_( AstSkyFrame * );
int astTestAsTime_( AstSkyFrame *, int );
int astTestEquinox_( AstSkyFrame * );
int astTestNegLon_( AstSkyFrame * );
int astTestProjection_( AstSkyFrame * );
void astClearAsTime_( AstSkyFrame *, int );
void astClearEquinox_( AstSkyFrame * );
void astClearNegLon_( AstSkyFrame * );
void astClearProjection_( AstSkyFrame * );
void astSetAsTime_( AstSkyFrame *, int, int );
void astSetEquinox_( AstSkyFrame *, double );
void astSetNegLon_( AstSkyFrame *, int );
void astSetProjection_( AstSkyFrame *, const char * );
#endif

/* Function interfaces. */
/* ==================== */
/* These macros are wrap-ups for the functions defined by this class
   to make them easier to invoke (e.g. to avoid type mis-matches when
   passing pointers to objects from derived classes). */

/* Interfaces to standard class functions. */
/* --------------------------------------- */
/* Some of these functions provide validation, so we cannot use them
   to validate their own arguments. We must use a cast when passing
   object pointers (so that they can accept objects from derived
   classes). */

/* Check class membership. */
#define astCheckSkyFrame(this) astINVOKE_CHECK(SkyFrame,this)

/* Test class membership. */
#define astIsASkyFrame(this) astINVOKE_ISA(SkyFrame,this)

/* Constructor. */
#if defined(astCLASS)            /* Protected */
#define astSkyFrame astINVOKE(F,astSkyFrame_)
#else
#define astSkyFrame astINVOKE(F,astSkyFrameId_)
#endif

#if defined(astCLASS)            /* Protected */

/* Initialiser. */
#define astInitSkyFrame(mem,size,init,vtab,name) \
astINVOKE(O,astInitSkyFrame_(mem,size,init,vtab,name))

/* Vtab Initialiser. */
#define astInitSkyFrameVtab(vtab,name) astINVOKE(V,astInitSkyFrameVtab_(vtab,name))
/* Loader. */
#define astLoadSkyFrame(mem,size,vtab,name,channel) \
astINVOKE(O,astLoadSkyFrame_(mem,size,vtab,name,astCheckChannel(channel)))

#endif

/* Interfaces to public member functions. */
/* -------------------------------------- */

/* None. */

/* Interfaces to protected member functions. */
/* ----------------------------------------- */
/* Here we make use of astCheckSkyFrame to validate SkyFrame pointers
   before use. This provides a contextual error report if a pointer to
   the wrong sort of object is supplied. */

#if defined(astCLASS)            /* Protected */
#define astClearAsTime(this,axis) \
astINVOKE(V,astClearAsTime_(astCheckSkyFrame(this),axis))
#define astClearEquinox(this) \
astINVOKE(V,astClearEquinox_(astCheckSkyFrame(this)))
#define astClearNegLon(this) \
astINVOKE(V,astClearNegLon_(astCheckSkyFrame(this)))
#define astClearProjection(this) \
astINVOKE(V,astClearProjection_(astCheckSkyFrame(this)))
#define astGetAsTime(this,axis) \
astINVOKE(V,astGetAsTime_(astCheckSkyFrame(this),axis))
#define astGetEquinox(this) \
astINVOKE(V,astGetEquinox_(astCheckSkyFrame(this)))
#define astGetNegLon(this) \
astINVOKE(V,astGetNegLon_(astCheckSkyFrame(this)))
#define astGetLatAxis(this) \
astINVOKE(V,astGetLatAxis_(astCheckSkyFrame(this)))
#define astGetLonAxis(this) \
astINVOKE(V,astGetLonAxis_(astCheckSkyFrame(this)))
#define astGetProjection(this) \
astINVOKE(V,astGetProjection_(astCheckSkyFrame(this)))
#define astSetAsTime(this,axis,value) \
astINVOKE(V,astSetAsTime_(astCheckSkyFrame(this),axis,value))
#define astSetEquinox(this,value) \
astINVOKE(V,astSetEquinox_(astCheckSkyFrame(this),value))
#define astSetNegLon(this,value) \
astINVOKE(V,astSetNegLon_(astCheckSkyFrame(this),value))
#define astSetProjection(this,value) \
astINVOKE(V,astSetProjection_(astCheckSkyFrame(this),value))
#define astTestAsTime(this,axis) \
astINVOKE(V,astTestAsTime_(astCheckSkyFrame(this),axis))
#define astTestEquinox(this) \
astINVOKE(V,astTestEquinox_(astCheckSkyFrame(this)))
#define astTestNegLon(this) \
astINVOKE(V,astTestNegLon_(astCheckSkyFrame(this)))
#define astTestProjection(this) \
astINVOKE(V,astTestProjection_(astCheckSkyFrame(this)))
#endif
#endif
