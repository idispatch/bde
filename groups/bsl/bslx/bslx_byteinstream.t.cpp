// bslx_byteinstream.t.cpp                                            -*-C++-*-

#include <bslx_byteinstream.h>
#include <bslx_byteoutstream.h>                 // for testing only

#include <bsl_cstdlib.h>     // atoi()
#include <bsl_cstring.h>     // memcpy(), memcmp(), strlen()
#include <bsl_iostream.h>
#include <bsl_iomanip.h>
#include <bsl_strstream.h>

using namespace BloombergLP;
using namespace bsl;  // automatically added by script
using namespace bslx;


//=============================================================================
//                    STANDARD BDE ASSERT TEST MACRO
//-----------------------------------------------------------------------------
static int testStatus = 0;
static void aSsErT(int c, const char *s, int i)
{
    if (c) {
        cout << "Error " << __FILE__ << "(" << i << "): " << s
             << "    (failed)" << endl;
        if (testStatus >= 0 && testStatus <= 100) ++testStatus;
    }
}
#define ASSERT(X) { aSsErT(!(X), #X, __LINE__); }
//-----------------------------------------------------------------------------
#define LOOP_ASSERT(I,X) { \
    if (!(X)) { cout << #I << ": " << I << "\n"; aSsErT(1, #X, __LINE__);}}

#define LOOP2_ASSERT(I,J,X) { \
    if (!(X)) { cout << #I << ": " << I << "\t" << #J << ": " \
        << J << "\n"; aSsErT(1, #X, __LINE__); } }

//=============================================================================
//                              TEST PLAN
//-----------------------------------------------------------------------------
//                              Overview
//                              --------
// For all input methods in 'ByteInStream', the "unexternalization" from
// byte representation to the correct fundamental-type value is delegated to
// another component.  We assume that this process has been rigorously tested
// and verified.  Therefore, we are concerned only with the placement of the
// next-byte-position cursor and the alignment of bytes in the input stream.
// For each input method, we verify these properties by first creating a
// 'ByteOutStream' object 'oX' containing some arbitrarily chosen values.
// The values are interleaved with chosen "marker" bytes to check for alignment
// issues.  We then create a 'ByteInStream' object 'iX' initialized with
// the content of 'oX', and consecutively call the input method on 'iX' to
// verify that the extracted values and marker bytes equal to their respective
// chosen values.  After all values are extracted, we verify that the stream is
// valid, empty, and the cursor is properly placed.
//-----------------------------------------------------------------------------
// [ 2] ByteInStream();
// [ 2] ByteInStream(const char *buffer, int numBytes);
// [ 2] ~ByteInStream();
// [25] getLength(int& variable);
// [25] getVersion(int& variable);
// [12] getInt64(bsls::Types::Int64& variable);
// [12] getUint64(bsls::Types::Uint64& variable);
// [11] getInt56(bsls::Types::Int64& variable);
// [11] getUint56(bsls::Types::Uint64& variable);
// [10] getInt48(bsls::Types::Int64& variable);
// [10] getUint48(bsls::Types::Uint64& variable);
// [ 9] getInt40(bsls::Types::Int64& variable);
// [ 9] getUint40(bsls::Types::Uint64& variable);
// [ 8] getInt32(int& variable);
// [ 8] getUint32(unsigned int& variable);
// [ 7] getInt24(int& variable);
// [ 7] getUint24(unsigned int& variable);
// [ 6] getInt16(short& variable);
// [ 6] getUint16(unsigned short& variable);
// [ 2] getInt8(char& variable);
// [ 5] getInt8(signed char& variable);
// [ 5] getUint8(char& variable);
// [ 5] getUint8(unsigned char& variable);
// [14] getFloat64(double& variable);
// [13] getFloat32(float& variable);
// [22] getArrayInt64(bsls::Types::Int64 *values, int numValues);
// [22] getArrayUint64(bsls::Types::Uint64 *values, int numValues);
// [21] getArrayInt56(bsls::Types::Int64 *values, int numValues);
// [21] getArrayUint56(bsls::Types::Uint64 *values, int numValues);
// [20] getArrayInt48(bsls::Types::Int64 *values, int numValues);
// [20] getArrayUint48(bsls::Types::Uint64 *values, int numValues);
// [19] getArrayInt40(bsls::Types::Int64 *values, int numValues);
// [19] getArrayUint40(bsls::Types::Uint64 *values, int numValues);
// [18] getArrayInt32(int *values, int numValues);
// [18] getArrayUint32(unsigned int *values, int numValues);
// [17] getArrayInt24(int *values, int numValues);
// [17] getArrayUint24(unsigned int *values, int numValues);
// [16] getArrayInt16(short *values, int numValues);
// [16] getArrayUint16(unsigned short *values,int numValues);
// [15] getArrayInt8(char *values, int numValues);
// [15] getArrayInt8(signed char *values, int numValues);
// [15] getArrayUint8(char *values, int numValues);
// [15] getArrayUint8(unsigned char *values, int numValues);
// [24] getArrayFloat64(double *values, int numValues);
// [23] getArrayFloat32(float *values, int numValues);
// [ 2] void invalidate();
// [ 3] operator const void *() const;
// [ 3] bool isValid() const;
// [ 3] const char *data() const;
// [ 3] bool isEmpty() const;
// [ 3] int length() const;
// [ 3] int cursor() const;
//
// [ 4] ostream& operator<<(ostream& stream, const ByteInStream& obj);
//-----------------------------------------------------------------------------
// [ 1] BREATHING TEST
// [26] USAGE EXAMPLE
//=============================================================================
#define P(X) cout << #X " = " << (X) << endl; // Print identifier and value.
#define Q(X) cout << "<| " #X " |>" << endl;  // Quote identifier literally.
#define P_(X) cout << #X " = " << (X) << ", " << flush; // P(X) without '\n'
#define L_ __LINE__                           // current Line number

//=============================================================================
//                  GLOBAL TYPEDEFS/CONSTANTS FOR TESTING
//-----------------------------------------------------------------------------

typedef ByteInStream Obj;
typedef ByteOutStream Out;

const int SIZEOF_INT64   = 8;
const int SIZEOF_INT32   = 4;
const int SIZEOF_INT16   = 2;
const int SIZEOF_INT8    = 1;
const int SIZEOF_FLOAT64 = 8;
const int SIZEOF_FLOAT32 = 4;

//=============================================================================
//              Classes, functions, etc., needed for Usage Example
//-----------------------------------------------------------------------------
// Suppose we wish to implement a (deliberately simple) 'MyPerson' class (in
// an appropriate 'myperson' component) as a value-semantic object that
// supports 'bslx' externalization.  In addition to whatever data and methods
// that we choose to put into our design, we must supply three methods of
// specific name and signature in order to comply with the 'bslx' "protocol":
// a class method 'isBslxVersionSupported' and two instance methods, an
// accessor (i.e., a 'const' method) 'bslxStreamOut', and a manipulator (i.e.,
// a non-'const' method) 'bslxStreamIn'.  This example shows how to implement
// those three methods for the simple "person" component.
//
// In this example we will not worry overly about "good design" of the person
// component, and we will declare but not implement the suite of value-semantic
// methods and free operators.  In particular, we will not make explicit use of
// 'bslma' allocators; a more complete design would do so.
//..
   // myperson.h

   class MyPerson {
       bsl::string d_firstName;
       bsl::string d_lastName;
       int         d_age;

       friend bool operator==(const MyPerson&, const MyPerson&);

     public:
       // CLASS METHODS
       static bool isBslxVersionSupported(int version) { return 1 == version; }
           // Return 'true' if the specified 'version' is supported by
           // this class, and 'flase' otherwise.

       // CREATORS
       MyPerson();
           // Create a default person.

       MyPerson(const char *firstName, const char *lastName, int age);
           // Create a person object having the specified 'firstName',
           // 'lastName', and 'age'.

       MyPerson(const MyPerson& original);
           // Create a person object having value of the specified 'original'
           // person.

       ~MyPerson();
           // Destroy this object.

       // MANIPULATORS
       MyPerson& operator=(const MyPerson& rhs);
           // Assign to this person object the value of the specified 'rhs'
           // person.

       template <class STREAM>
       STREAM& bslxStreamIn(STREAM& stream, int version);
           // Assign to this object the value read from the specified input
           // 'stream' using the specified 'version' format and return a
           // reference to the modifiable 'stream'.  If 'stream' is initially
           // invalid, this operation has no effect.  If 'stream' becomes
           // invalid during this operation, this object is valid, but its
           // value is undefined.  If the specified 'version' is not supported,
           // 'stream' is marked invalid, but this object is unaltered.  Note
           // that no version is read from 'stream'.  (See the 'bslx'
           // package-level documentation for more information on 'bslx'
           // streaming of container types.)

       // Other manipulators omitted.

       // ACCESSORS
       const bsl::string& firstName() const;
           // Return the first name of this person.

       const bsl::string& lastName() const;
           // Return the last name of this person.

       int age() const;
           // Return the age of this person.

       template <class STREAM>
       STREAM& bslxStreamOut(STREAM& stream, int version) const;
           // Write this value to the specified output 'stream' and return a
           // reference to the modifiable 'stream'.  Optionally specify an
           // explicit 'version' format; by default, the maximum supported
           // version is written to 'stream' and used as the format.  If
           // 'version' is specified, that format is used but *not* written to
           // 'stream'.  If 'version' is not supported, 'stream' is left
           // unmodified.  (See the 'bslx' package-level documentation for more
           // information on 'bslx' streaming of container types).

       // Other accessors omitted.

   };

   // FREE OPERATORS
   inline
   bool operator==(const MyPerson& lhs, const MyPerson& rhs);
       // Return 'true' if the specified 'lhs' and 'rhs' person objects have
       // the same value and 'false' otherwise.  Two person objects have the
       // same value if they have the same first name, last name, and age.

   inline
   bool operator!=(const MyPerson& lhs, const MyPerson& rhs);
       // Return 'true' if the specified 'lhs' and 'rhs' person objects do not
       // have the same value and 'false' otherwise.  Two person objects differ
       // in value if they differ in first name, last name, or age.

   bsl::ostream& operator<<(bsl::ostream& stream, const MyPerson& person);
       // Write the specified 'date' value to the specified output 'stream' in
       // some reasonable format.

   // INLINE FUNCTION DEFINITIONS
   inline
   MyPerson::MyPerson()
   : d_firstName("")
   , d_lastName("")
   , d_age(0)
   {
   }

   inline
   MyPerson::MyPerson(const char *firstName, const char *lastName, int age)
   : d_firstName(firstName)
   , d_lastName(lastName)
   , d_age(age)
   {
   }

   inline
   MyPerson::~MyPerson()
   {
   }

   template <class STREAM>
   inline
   STREAM& MyPerson::bslxStreamIn(STREAM& stream, int version)
   {
       if (stream) {
           switch (version) {    // switch on the 'bslx' version
             case 1: {
                 stream.getString(d_firstName);
                 if (!stream) {
                     d_firstName = "stream error";  // *might* be corrupted;
                                                    //  value for testing
                     return stream;
                 }
                 stream.getString(d_lastName);
                 if (!stream) {
                     d_lastName = "stream error";  // *might* be corrupted;
                                                   //  value for testing
                     return stream;
                 }
                 stream.getInt32(d_age);
                 if (!stream) {
                     d_age = 1;      // *might* be corrupted; value for testing
                     return stream;
                 }
             } break;
             default: {
               stream.invalidate();
             }
           }
       }
       return stream;
   }

   // ACCESSORS
   const bsl::string& MyPerson::firstName() const
   {
       return d_firstName;
   }

   const bsl::string& MyPerson::lastName() const
   {
       return d_lastName;
   }

   int MyPerson::age() const
   {
       return d_age;
   }

   template <class STREAM>
   inline
   STREAM& MyPerson::bslxStreamOut(STREAM& stream, int version) const
   {
       switch (version) {
         case 1: {
             stream.putString(d_firstName);
             stream.putString(d_lastName);
             stream.putInt32(d_age);
         } break;
       }
       return stream;
   }

   // FREE OPERATORS
   inline
   bool operator==(const MyPerson& lhs, const MyPerson& rhs)
   {
       return lhs.d_firstName == rhs.d_firstName &&
              lhs.d_lastName  == rhs.d_lastName  &&
              lhs.d_age       == rhs.d_age;
   }

   inline
   bool operator!=(const MyPerson& lhs, const MyPerson& rhs)
   {
       return !(lhs == rhs);
   }

//=============================================================================
//                                 MAIN PROGRAM
//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    int test = argc > 1 ? atoi(argv[1]) : 0;
    int verbose = argc > 2;
    int veryVerbose = argc > 3;

    cout << "TEST " << __FILE__ << " CASE " << test << endl;

    switch (test) { case 0:
      case 26: {
        // --------------------------------------------------------------------
        // TESTING USAGE EXAMPLE
        //   The usage example provided in the component header file must
        //   compile, link, and run on all platforms as shown.
        //
        // Plan:
        //   Incorporate usage example from header into driver, remove leading
        //   comment characters, and replace 'assert' with 'ASSERT'.
        //
        // Testing:
        //   USAGE EXAMPLE
        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting Usage Example"
                 << "\n=====================" << endl;
        }

// my_testapp.m.cpp

// using namespace bsl;

// int main(int argc, char **argv) {
   {
       MyPerson JaneSmith("Jane", "Smith", 42);
       const int VERSION = 1;
       ByteOutStream outStream;
       outStream.putVersion(VERSION);
       JaneSmith.bslxStreamOut(outStream, VERSION);

       MyPerson janeCopy;
       ASSERT(janeCopy != JaneSmith);

       ByteInStream inStream(outStream.data(), outStream.length());
       int version;
       inStream.getVersion(version);
       ASSERT(version == VERSION);
       janeCopy.bslxStreamIn(inStream, version);
       ASSERT(inStream);

       ASSERT(janeCopy == JaneSmith);

       // Verify the results on 'stdout'.
       if (verbose) {
           if (janeCopy == JaneSmith) {
               cout << "Successfully serialized and de-serialized Jane Smith:"
                    << "\n\tFirstName: " << janeCopy.firstName()
                    << "\n\tLastName : " << janeCopy.lastName()
                    << "\n\tAge      : " << janeCopy.age() << endl;
           }
           else {
               cout << "Serialization unsuccessful.  'janeCopy' holds:"
                    << "\n\tFirstName: " << janeCopy.firstName()
                    << "\n\tLastName : " << janeCopy.lastName()
                    << "\n\tAge      : " << janeCopy.age() << endl;
           }
       }
//     return 0;
   }

      } break;
      case 25: {
        // --------------------------------------------------------------------
        // GET LENGTH AND VERSION:
        //
        // Testing:
        //   getLength(int& variable);
        //   getVersion(int& variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET LENGTH AND VERSION TEST" << endl
                 << "===========================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getLength(int&)." << endl;
        }
        {
            Out o;
            o.putLength(1);             o.putInt8(0xFF);
            o.putLength(128);           o.putInt8(0xFE);
            o.putLength(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
              if (veryVerbose) { P(x) }
              char marker;
              int val;
              x.getLength(val);          x.getInt8(marker);
              ASSERT(1 == val);          ASSERT('\xFF' == marker);
              x.getLength(val);          x.getInt8(marker);
              ASSERT(128 == val);        ASSERT('\xFE' == marker);
              x.getLength(val);          x.getInt8(marker);
              ASSERT(3 == val);          ASSERT('\xFD' == marker);
              ASSERT(x);
              ASSERT(x.isEmpty());
              ASSERT(x.cursor() == x.length());
          }
          {
              Out o;
              o.putLength(128);             o.putInt8(0xFD);
              o.putLength(127);             o.putInt8(0xFE);
              o.putLength(256);             o.putInt8(0xFF);

              Obj x(o.data(), o.length());
              if (veryVerbose) { P(x) }
              char marker;
              int val;
              x.getLength(val);          x.getInt8(marker);
              ASSERT(128 == val);        ASSERT('\xFD' == marker);
              x.getLength(val);          x.getInt8(marker);
              ASSERT(127 == val);        ASSERT('\xFE' == marker);
              x.getLength(val);          x.getInt8(marker);
              ASSERT(256 == val);        ASSERT('\xFF' == marker);
              ASSERT(x);
              ASSERT(x.isEmpty());
              ASSERT(x.cursor() == x.length());
          }

          if (verbose) {
              cout << "\nTesting getVersion(int&)." << endl;
          }
          {
              Out o;
              o.putVersion(1);
              o.putVersion(2);
              o.putVersion(3);
              o.putVersion(4);

              Obj x(o.data(), o.length());
              if (veryVerbose) { P(x) }
              int val;
              x.getVersion(val);            ASSERT(1 == val);
              x.getVersion(val);            ASSERT(2 == val);
              x.getVersion(val);            ASSERT(3 == val);
              x.getVersion(val);            ASSERT(4 == val);
              ASSERT(x);
              ASSERT(x.isEmpty());
              ASSERT(x.cursor() == x.length());
          }
          {
              Out o;
              o.putVersion(252);
              o.putVersion(253);
              o.putVersion(254);
              o.putVersion(255);

              Obj x(o.data(), o.length());
              if (veryVerbose) { P(x) }
              int val;
              x.getVersion(val);            ASSERT(252 == val);
              x.getVersion(val);            ASSERT(253 == val);
              x.getVersion(val);            ASSERT(254 == val);
              x.getVersion(val);            ASSERT(255 == val);
              ASSERT(x);
              ASSERT(x.isEmpty());
              ASSERT(x.cursor() == x.length());
          }
      } break;
      case 24: {
        // --------------------------------------------------------------------
        // GET 64-BIT FLOAT ARRAYS TEST:
        //
        // Testing:
        //   getArrayFloat64(double *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                          << "GET 64-BIT FLOAT ARRAYS TEST" << endl
                          << "============================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getArrayFloat64." << endl;
        }
        {
            const double DATA[] = { 1, 2, 3 };
            const double V = 0xFF;

            Out o;
            o.putArrayFloat64(DATA, 0);             o.putInt8(0xFF);
            o.putArrayFloat64(DATA, 1);             o.putInt8(0xFE);
            o.putArrayFloat64(DATA, 2);             o.putInt8(0xFD);
            o.putArrayFloat64(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            double ar[] = { V, V, V };
            x.getArrayFloat64(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayFloat64(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayFloat64(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayFloat64(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const double DATA[] = { 4, 5, 6 };
            const double V = 0xFF;

            Out o;
            o.putArrayFloat64(DATA, 0);             o.putInt8(0xFF);
            o.putArrayFloat64(DATA, 1);             o.putInt8(0xFE);
            o.putArrayFloat64(DATA, 2);             o.putInt8(0xFD);
            o.putArrayFloat64(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            double ar[] = { V, V, V };
            x.getArrayFloat64(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayFloat64(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayFloat64(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayFloat64(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 23: {
        // --------------------------------------------------------------------
        // GET 32-BIT FLOAT ARRAYS TEST:
        //
        // Testing:
        //   getArrayFloat32(float *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 32-BIT FLOAT ARRAYS TEST" << endl
                 << "============================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getArrayFloat32." << endl;
        }
        {
            const float DATA[] = { 1, 2, 3 };
            const float V = 0xFF;

            Out o;
            o.putArrayFloat32(DATA, 0);             o.putInt8(0xFF);
            o.putArrayFloat32(DATA, 1);             o.putInt8(0xFE);
            o.putArrayFloat32(DATA, 2);             o.putInt8(0xFD);
            o.putArrayFloat32(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            float ar[] = { V, V, V };
            x.getArrayFloat32(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayFloat32(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayFloat32(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayFloat32(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const float DATA[] = { 4, 5, 6 };
            const float V = 0xFF;

            Out o;
            o.putArrayFloat32(DATA, 0);             o.putInt8(0xFF);
            o.putArrayFloat32(DATA, 1);             o.putInt8(0xFE);
            o.putArrayFloat32(DATA, 2);             o.putInt8(0xFD);
            o.putArrayFloat32(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            float ar[] = { V, V, V };
            x.getArrayFloat32(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayFloat32(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayFloat32(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayFloat32(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 22: {
        // --------------------------------------------------------------------
        // GET 64-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   getArrayInt64(bsls::Types::Int64 *values, int numValues);
        //   getArrayUint64(bsls::Types::Uint64 *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 64-BIT INTEGER ARRAYS TEST" << endl
                 << "==============================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getArrayInt64." << endl;
        }
        {
            const bsls::Types::Int64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt64(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt64(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt64(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt64(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt64(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt64(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt64(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt64(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Int64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt64(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt64(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt64(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt64(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt64(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt64(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt64(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt64(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint64." << endl;
        }
        {
            const bsls::Types::Uint64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint64(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint64(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint64(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint64(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint64(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint64(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint64(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint64(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Uint64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint64(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint64(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint64(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint64(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint64(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint64(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint64(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint64(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 21: {
        // --------------------------------------------------------------------
        // GET 56-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   getArrayInt56(bsls::Types::Int64 *values, int numValues);
        //   getArrayUint56(bsls::Types::Uint64 *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 56-BIT INTEGER ARRAYS TEST" << endl
                 << "==============================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getArrayInt56." << endl;
        }
        {
            const bsls::Types::Int64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt56(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt56(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt56(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt56(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt56(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt56(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt56(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt56(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Int64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt56(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt56(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt56(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt56(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt56(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt56(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt56(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt56(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint56." << endl;
        }
        {
            const bsls::Types::Uint64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint56(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint56(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint56(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint56(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint56(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint56(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint56(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint56(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Uint64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint56(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint56(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint56(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint56(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint56(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint56(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint56(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint56(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 20: {
        // --------------------------------------------------------------------
        // GET 48-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   getArrayInt48(bsls::Types::Int64 *values, int numValues);
        //   getArrayUint48(bsls::Types::Uint64 *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 48-BIT INTEGER ARRAYS TEST" << endl
                 << "==============================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getArrayInt48." << endl;
        }
        {
            const bsls::Types::Int64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt48(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt48(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt48(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt48(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt48(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt48(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt48(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt48(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Int64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt48(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt48(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt48(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt48(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt48(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt48(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt48(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt48(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint48." << endl;
        }
        {
            const bsls::Types::Uint64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint48(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint48(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint48(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint48(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint48(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint48(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint48(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint48(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Uint64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint48(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint48(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint48(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint48(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint48(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint48(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint48(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint48(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 19: {
        // --------------------------------------------------------------------
        // GET 40-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   getArrayInt40(bsls::Types::Int64 *values, int numValues);
        //   getArrayUint40(bsls::Types::Uint64 *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 40-BIT INTEGER ARRAYS TEST" << endl
                 << "==============================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getArrayInt40." << endl;
        }
        {
            const bsls::Types::Int64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt40(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt40(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt40(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt40(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt40(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt40(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt40(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt40(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Int64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Int64 V = 0xFF;

            Out o;
            o.putArrayInt40(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt40(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt40(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt40(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 ar[] = { V, V, V };
            x.getArrayInt40(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt40(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt40(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt40(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint40." << endl;
        }
        {
            const bsls::Types::Uint64 DATA[] = { 1, 2, 3 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint40(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint40(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint40(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint40(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint40(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint40(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint40(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint40(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const bsls::Types::Uint64 DATA[] = { 4, 5, 6 };
            const bsls::Types::Uint64 V = 0xFF;

            Out o;
            o.putArrayUint40(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint40(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint40(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint40(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 ar[] = { V, V, V };
            x.getArrayUint40(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint40(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint40(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint40(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 18: {
        // --------------------------------------------------------------------
        // GET 32-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   getArrayInt32(int *values, int numValues);
        //   getArrayUint32(unsigned int *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 32-BIT INTEGER ARRAYS TEST" << endl
                 << "==============================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getArrayInt32." << endl;
        }
        {
            const int DATA[] = { 1, 2, 3 };
            const int V = 0xFF;

            Out o;
            o.putArrayInt32(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt32(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt32(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt32(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int ar[] = { V, V, V };
            x.getArrayInt32(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt32(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt32(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt32(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const int DATA[] = { 4, 5, 6 };
            const int V = 0xFF;

            Out o;
            o.putArrayInt32(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt32(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt32(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt32(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int ar[] = { V, V, V };
            x.getArrayInt32(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt32(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt32(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt32(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint32." << endl;
        }
        {
            const unsigned int DATA[] = { 1, 2, 3 };
            const unsigned int V = 0xFF;

            Out o;
            o.putArrayUint32(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint32(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint32(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint32(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int ar[] = { V, V, V };
            x.getArrayUint32(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint32(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint32(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint32(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const unsigned int DATA[] = { 4, 5, 6 };
            const unsigned int V = 0xFF;

            Out o;
            o.putArrayUint32(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint32(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint32(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint32(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int ar[] = { V, V, V };
            x.getArrayUint32(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint32(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint32(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint32(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 17: {
        // --------------------------------------------------------------------
        // GET 24-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   getArrayInt24(int *values, int numValues);
        //   getArrayUint24(unsigned int *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 24-BIT INTEGER ARRAYS TEST" << endl
                 << "==============================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getArrayInt24." << endl;
        }
        {
            const int DATA[] = { 1, 2, 3 };
            const int V = 0xFF;

            Out o;
            o.putArrayInt24(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt24(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt24(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt24(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int ar[] = { V, V, V };
            x.getArrayInt24(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt24(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt24(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt24(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const int DATA[] = { 4, 5, 6 };
            const int V = 0xFF;

            Out o;
            o.putArrayInt24(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt24(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt24(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt24(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int ar[] = { V, V, V };
            x.getArrayInt24(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt24(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt24(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt24(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint24." << endl;
        }
        {
            const unsigned int DATA[] = { 1, 2, 3 };
            const unsigned int V = 0xFF;

            Out o;
            o.putArrayUint24(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint24(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint24(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint24(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int ar[] = { V, V, V };
            x.getArrayUint24(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint24(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint24(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint24(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const unsigned int DATA[] = { 4, 5, 6 };
            const unsigned int V = 0xFF;

            Out o;
            o.putArrayUint24(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint24(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint24(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint24(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int ar[] = { V, V, V };
            x.getArrayUint24(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint24(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint24(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint24(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 16: {
        // --------------------------------------------------------------------
        // GET 16-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //   getArrayInt16(short *values, int numValues);
        //   getArrayUint16(unsigned short *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 16-BIT INTEGER ARRAYS TEST" << endl
                 << "==============================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getArrayInt16." << endl;
        }
        {
            const short DATA[] = { 1, 2, 3 };
            const short V = 0xFF;

            Out o;
            o.putArrayInt16(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt16(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt16(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt16(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            short ar[] = { V, V, V };
            x.getArrayInt16(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt16(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt16(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt16(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const short DATA[] = { 4, 5, 6 };
            const short V = 0xFF;

            Out o;
            o.putArrayInt16(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt16(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt16(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt16(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            short ar[] = { V, V, V };
            x.getArrayInt16(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt16(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt16(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt16(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint16." << endl;
        }
        {
            const unsigned short DATA[] = { 1, 2, 3 };
            const unsigned short V = 0xFF;

            Out o;
            o.putArrayUint16(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint16(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint16(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint16(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned short ar[] = { V, V, V };
            x.getArrayUint16(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint16(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint16(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint16(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const unsigned short DATA[] = { 4, 5, 6 };
            const unsigned short V = 0xFF;

            Out o;
            o.putArrayUint16(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint16(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint16(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint16(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned short ar[] = { V, V, V };
            x.getArrayUint16(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint16(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint16(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint16(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 15: {
        // --------------------------------------------------------------------
        // GET 8-BIT INTEGER ARRAYS TEST:
        //
        // Testing:
        //    getArrayInt8(char *values, int numValues);
        //    getArrayInt8(signed char *values, int numValues);
        //    getArrayUint8(char *values, int numValues);
        //    getArrayUint8(unsigned char *values, int numValues);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 8-BIT INTEGER ARRAYS TEST" << endl
                 << "=============================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getArrayInt8(char*, int)." << endl;
        }
        {
            const char DATA[] = { 1, 2, 3 };
            const char V = (char) 0xFF;

            Out o;
            o.putArrayInt8(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt8(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt8(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt8(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            char ar[] = { V, V, V };
            x.getArrayInt8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const char DATA[] = { 4, 5, 6 };
            const char V = (char) 0xFF;

            Out o;
            o.putArrayInt8(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt8(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt8(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt8(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            char ar[] = { V, V, V };
            x.getArrayInt8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "Testing getArrayInt8(signed char*, int)." << endl;
        }
        {
            const signed char DATA[] = { 1, 2, 3 };
            const signed char V = (char) 0xFF;

            Out o;
            o.putArrayInt8(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt8(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt8(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt8(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            signed char ar[] = { V, V, V };
            x.getArrayInt8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const signed char DATA[] = { 4, 5, 6 };
            const signed char V = (char) 0xFF;

            Out o;
            o.putArrayInt8(DATA, 0);             o.putInt8(0xFF);
            o.putArrayInt8(DATA, 1);             o.putInt8(0xFE);
            o.putArrayInt8(DATA, 2);             o.putInt8(0xFD);
            o.putArrayInt8(DATA, 3);             o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            signed char ar[] = { V, V, V };
            x.getArrayInt8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayInt8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayInt8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayInt8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint8(char*, int)." << endl;
        }
        {
            const char DATA[] = { 1, 2, 3 };
            const char V = (char) 0xFF;

            Out o;
            o.putArrayUint8(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint8(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint8(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint8(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            char ar[] = { V, V, V };
            x.getArrayUint8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const char DATA[] = { 4, 5, 6 };
            const char V = (char) 0xFF;

            Out o;
            o.putArrayUint8(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint8(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint8(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint8(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            char ar[] = { V, V, V };
            x.getArrayUint8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getArrayUint8(unsigned char*, int)." << endl;
        }
        {
            const unsigned char DATA[] = { 1, 2, 3 };
            const unsigned char V = 0xFF;

            Out o;
            o.putArrayUint8(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint8(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint8(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint8(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned char ar[] = { V, V, V };
            x.getArrayUint8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            const unsigned char DATA[] = { 4, 5, 6 };
            const unsigned char V = 0xFF;

            Out o;
            o.putArrayUint8(DATA, 0);            o.putInt8(0xFF);
            o.putArrayUint8(DATA, 1);            o.putInt8(0xFE);
            o.putArrayUint8(DATA, 2);            o.putInt8(0xFD);
            o.putArrayUint8(DATA, 3);            o.putInt8(0xFC);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned char ar[] = { V, V, V };
            x.getArrayUint8(ar, 0);
            ASSERT(V == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFF' == marker);

            x.getArrayUint8(ar, 1);
            ASSERT(DATA[0] == ar[0] && V == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFE' == marker);

            x.getArrayUint8(ar, 2);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && V == ar[2]);
            x.getInt8(marker);            ASSERT('\xFD' == marker);

            x.getArrayUint8(ar, 3);
            ASSERT(DATA[0] == ar[0] && DATA[1] == ar[1] && DATA[2] == ar[2]);
            x.getInt8(marker);            ASSERT('\xFC' == marker);

            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 14: {
        // --------------------------------------------------------------------
        // GET 64-BIT FLOATS TEST:
        //
        // Testing:
        //   getFloat64(double &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 64-BIT FLOATS TEST" << endl
                 << "======================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getFloat64." << endl;
        }
        {
            Out o;
            o.putFloat64(1);           o.putInt8(0xFF);
            o.putFloat64(2);           o.putInt8(0xFE);
            o.putFloat64(3);           o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            double val;
            x.getFloat64(val);         x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getFloat64(val);         x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getFloat64(val);         x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putFloat64(4);           o.putInt8(0xFD);
            o.putFloat64(5);           o.putInt8(0xFE);
            o.putFloat64(6);           o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            double val;
            x.getFloat64(val);         x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getFloat64(val);         x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getFloat64(val);         x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 13: {
        // --------------------------------------------------------------------
        // GET 32-BIT FLOATS TEST:
        //
        // Testing:
        //   getFloat32(float &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 32-BIT FLOATS TEST" << endl
                 << "======================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getFloat32." << endl;
        }
        {
            Out o;
            o.putFloat32(1);           o.putInt8(0xFF);
            o.putFloat32(2);           o.putInt8(0xFE);
            o.putFloat32(3);           o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            float val;
            x.getFloat32(val);         x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getFloat32(val);         x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getFloat32(val);         x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putFloat32(4);           o.putInt8(0xFD);
            o.putFloat32(5);           o.putInt8(0xFE);
            o.putFloat32(6);           o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            float val;
            x.getFloat32(val);         x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getFloat32(val);         x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getFloat32(val);         x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 12: {
        // --------------------------------------------------------------------
        // GET 64-BIT INTEGERS TEST:
        //
        // Testing:
        //   getInt64(bsls::Types::Int64 val &variable);
        //   getUint64(bsls::Types::Uint64 val &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 64-BIT INTEGERS TEST" << endl
                 << "========================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getInt64." << endl;
        }
        {
            Out o;
            o.putInt64(1);             o.putInt8(0xFF);
            o.putInt64(2);             o.putInt8(0xFE);
            o.putInt64(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt64(val);           x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getInt64(val);           x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getInt64(val);           x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt64(4);             o.putInt8(0xFD);
            o.putInt64(5);             o.putInt8(0xFE);
            o.putInt64(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt64(val);           x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getInt64(val);           x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getInt64(val);           x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint64." << endl;
        }
        {
            Out o;
            o.putUint64(1);             o.putInt8(0xFF);
            o.putUint64(2);             o.putInt8(0xFE);
            o.putUint64(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint64(val);           x.getInt8(marker);
            ASSERT(1 == val);           ASSERT('\xFF' == marker);
            x.getUint64(val);           x.getInt8(marker);
            ASSERT(2 == val);           ASSERT('\xFE' == marker);
            x.getUint64(val);           x.getInt8(marker);
            ASSERT(3 == val);           ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint64(4);             o.putInt8(0xFD);
            o.putUint64(5);             o.putInt8(0xFE);
            o.putUint64(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint64(val);           x.getInt8(marker);
            ASSERT(4 == val);           ASSERT('\xFD' == marker);
            x.getUint64(val);           x.getInt8(marker);
            ASSERT(5 == val);           ASSERT('\xFE' == marker);
            x.getUint64(val);           x.getInt8(marker);
            ASSERT(6 == val);           ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 11: {
        // --------------------------------------------------------------------
        // GET 56-BIT INTEGERS TEST:
        //
        // Testing:
        //   getInt56(bsls::Types::Int64 val &variable);
        //   getUint56(bsls::Types::Uint64 val &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 56-BIT INTEGERS TEST" << endl
                 << "========================" << endl;
        }
        if (verbose) {
            cout << "\nTesting getInt56." << endl;
        }
        {
            Out o;
            o.putInt56(1);             o.putInt8(0xFF);
            o.putInt56(2);             o.putInt8(0xFE);
            o.putInt56(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt56(val);           x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getInt56(val);           x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getInt56(val);           x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt56(4);             o.putInt8(0xFD);
            o.putInt56(5);             o.putInt8(0xFE);
            o.putInt56(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt56(val);           x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getInt56(val);           x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getInt56(val);           x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint56." << endl;
        }
        {
            Out o;
            o.putUint56(1);             o.putInt8(0xFF);
            o.putUint56(2);             o.putInt8(0xFE);
            o.putUint56(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint56(val);           x.getInt8(marker);
            ASSERT(1 == val);           ASSERT('\xFF' == marker);
            x.getUint56(val);           x.getInt8(marker);
            ASSERT(2 == val);           ASSERT('\xFE' == marker);
            x.getUint56(val);           x.getInt8(marker);
            ASSERT(3 == val);           ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint56(4);             o.putInt8(0xFD);
            o.putUint56(5);             o.putInt8(0xFE);
            o.putUint56(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint56(val);           x.getInt8(marker);
            ASSERT(4 == val);           ASSERT('\xFD' == marker);
            x.getUint56(val);           x.getInt8(marker);
            ASSERT(5 == val);           ASSERT('\xFE' == marker);
            x.getUint56(val);           x.getInt8(marker);
            ASSERT(6 == val);           ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 10: {
        // --------------------------------------------------------------------
        // GET 48-BIT INTEGERS TEST:
        //
        // Testing:
        //   getInt48(bsls::Types::Int64 val &variable);
        //   getUint48(bsls::Types::Uint64 val &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 48-BIT INTEGERS TEST" << endl
                 << "========================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getInt48." << endl;
        }
        {
            Out o;
            o.putInt48(1);             o.putInt8(0xFF);
            o.putInt48(2);             o.putInt8(0xFE);
            o.putInt48(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt48(val);           x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getInt48(val);           x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getInt48(val);           x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt48(4);             o.putInt8(0xFD);
            o.putInt48(5);             o.putInt8(0xFE);
            o.putInt48(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt48(val);           x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getInt48(val);           x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getInt48(val);           x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint48." << endl;
        }
        {
            Out o;
            o.putUint48(1);             o.putInt8(0xFF);
            o.putUint48(2);             o.putInt8(0xFE);
            o.putUint48(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint48(val);           x.getInt8(marker);
            ASSERT(1 == val);           ASSERT('\xFF' == marker);
            x.getUint48(val);           x.getInt8(marker);
            ASSERT(2 == val);           ASSERT('\xFE' == marker);
            x.getUint48(val);           x.getInt8(marker);
            ASSERT(3 == val);           ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint48(4);             o.putInt8(0xFD);
            o.putUint48(5);             o.putInt8(0xFE);
            o.putUint48(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint48(val);           x.getInt8(marker);
            ASSERT(4 == val);           ASSERT('\xFD' == marker);
            x.getUint48(val);           x.getInt8(marker);
            ASSERT(5 == val);           ASSERT('\xFE' == marker);
            x.getUint48(val);           x.getInt8(marker);
            ASSERT(6 == val);           ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 9: {
        // --------------------------------------------------------------------
        // GET 40-BIT INTEGERS TEST:
        //
        // Testing:
        //   getInt40(bsls::Types::Int64 val &variable);
        //   getUint40(bsls::Types::Uint64 val &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 40-BIT INTEGERS TEST" << endl
                 << "========================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getInt40." << endl;
        }
        {
            Out o;
            o.putInt40(1);             o.putInt8(0xFF);
            o.putInt40(2);             o.putInt8(0xFE);
            o.putInt40(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt40(val);           x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getInt40(val);           x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getInt40(val);           x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt40(4);             o.putInt8(0xFD);
            o.putInt40(5);             o.putInt8(0xFE);
            o.putInt40(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Int64 val;
            x.getInt40(val);           x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getInt40(val);           x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getInt40(val);           x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint40." << endl;
        }
        {
            Out o;
            o.putUint40(1);             o.putInt8(0xFF);
            o.putUint40(2);             o.putInt8(0xFE);
            o.putUint40(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint40(val);           x.getInt8(marker);
            ASSERT(1 == val);           ASSERT('\xFF' == marker);
            x.getUint40(val);           x.getInt8(marker);
            ASSERT(2 == val);           ASSERT('\xFE' == marker);
            x.getUint40(val);           x.getInt8(marker);
            ASSERT(3 == val);           ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint40(4);             o.putInt8(0xFD);
            o.putUint40(5);             o.putInt8(0xFE);
            o.putUint40(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            bsls::Types::Uint64 val;
            x.getUint40(val);           x.getInt8(marker);
            ASSERT(4 == val);           ASSERT('\xFD' == marker);
            x.getUint40(val);           x.getInt8(marker);
            ASSERT(5 == val);           ASSERT('\xFE' == marker);
            x.getUint40(val);           x.getInt8(marker);
            ASSERT(6 == val);           ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 8: {
        // --------------------------------------------------------------------
        // GET 32-BIT INTEGERS TEST:
        //
        // Testing:
        //   getInt32(int &variable);
        //   getUint32(unsigned int &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 32-BIT INTEGERS TEST" << endl
                 << "========================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getInt32." << endl;
        }
        {
            Out o;
            o.putInt32(1);             o.putInt8(0xFF);
            o.putInt32(2);             o.putInt8(0xFE);
            o.putInt32(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int val;
            x.getInt32(val);           x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getInt32(val);           x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getInt32(val);           x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt32(4);             o.putInt8(0xFD);
            o.putInt32(5);             o.putInt8(0xFE);
            o.putInt32(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int val;
            x.getInt32(val);           x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getInt32(val);           x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getInt32(val);           x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint32." << endl;
        }
        {
            Out o;
            o.putUint32(1);             o.putInt8(0xFF);
            o.putUint32(2);             o.putInt8(0xFE);
            o.putUint32(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int val;
            x.getUint32(val);           x.getInt8(marker);
            ASSERT(1 == val);           ASSERT('\xFF' == marker);
            x.getUint32(val);           x.getInt8(marker);
            ASSERT(2 == val);           ASSERT('\xFE' == marker);
            x.getUint32(val);           x.getInt8(marker);
            ASSERT(3 == val);           ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint32(4);             o.putInt8(0xFD);
            o.putUint32(5);             o.putInt8(0xFE);
            o.putUint32(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int val;
            x.getUint32(val);           x.getInt8(marker);
            ASSERT(4 == val);           ASSERT('\xFD' == marker);
            x.getUint32(val);           x.getInt8(marker);
            ASSERT(5 == val);           ASSERT('\xFE' == marker);
            x.getUint32(val);           x.getInt8(marker);
            ASSERT(6 == val);           ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 7: {
        // --------------------------------------------------------------------
        // GET 24-BIT INTEGERS TEST:
        //
        // Testing:
        //   getInt24(int &variable);
        //   getUint24(unsigned int &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 24-BIT INTEGERS TEST" << endl
                 << "========================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getInt24." << endl;
        }
        {
            Out o;
            o.putInt24(1);             o.putInt8(0xFF);
            o.putInt24(2);             o.putInt8(0xFE);
            o.putInt24(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int val;
            x.getInt24(val);           x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getInt24(val);           x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getInt24(val);           x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt24(4);             o.putInt8(0xFD);
            o.putInt24(5);             o.putInt8(0xFE);
            o.putInt24(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            int val;
            x.getInt24(val);           x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getInt24(val);           x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getInt24(val);           x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint24." << endl;
        }
        {
            Out o;
            o.putUint24(1);             o.putInt8(0xFF);
            o.putUint24(2);             o.putInt8(0xFE);
            o.putUint24(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int val;
            x.getUint24(val);           x.getInt8(marker);
            ASSERT(1 == val);           ASSERT('\xFF' == marker);
            x.getUint24(val);           x.getInt8(marker);
            ASSERT(2 == val);           ASSERT('\xFE' == marker);
            x.getUint24(val);           x.getInt8(marker);
            ASSERT(3 == val);           ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint24(4);             o.putInt8(0xFD);
            o.putUint24(5);             o.putInt8(0xFE);
            o.putUint24(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned int val;
            x.getUint24(val);           x.getInt8(marker);
            ASSERT(4 == val);           ASSERT('\xFD' == marker);
            x.getUint24(val);           x.getInt8(marker);
            ASSERT(5 == val);           ASSERT('\xFE' == marker);
            x.getUint24(val);           x.getInt8(marker);
            ASSERT(6 == val);           ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 6: {
        // --------------------------------------------------------------------
        // GET 16-BIT INTEGERS TEST:
        //
        // Testing:
        //   getInt16(short &variable);
        //   getUint16(unsigned short &variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 16-BIT INTEGERS TEST" << endl
                 << "========================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getInt16." << endl;
        }
        {
            Out o;
            o.putInt16(1);             o.putInt8(0xFF);
            o.putInt16(2);             o.putInt8(0xFE);
            o.putInt16(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            short val;
            x.getInt16(val);           x.getInt8(marker);
            ASSERT(1 == val);          ASSERT('\xFF' == marker);
            x.getInt16(val);           x.getInt8(marker);
            ASSERT(2 == val);          ASSERT('\xFE' == marker);
            x.getInt16(val);           x.getInt8(marker);
            ASSERT(3 == val);          ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt16(4);             o.putInt8(0xFD);
            o.putInt16(5);             o.putInt8(0xFE);
            o.putInt16(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            short val;
            x.getInt16(val);           x.getInt8(marker);
            ASSERT(4 == val);          ASSERT('\xFD' == marker);
            x.getInt16(val);           x.getInt8(marker);
            ASSERT(5 == val);          ASSERT('\xFE' == marker);
            x.getInt16(val);           x.getInt8(marker);
            ASSERT(6 == val);          ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint16." << endl;
        }
        {
            Out o;
            o.putUint16(1);             o.putInt8(0xFF);
            o.putUint16(2);             o.putInt8(0xFE);
            o.putUint16(3);             o.putInt8(0xFD);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned short val;
            x.getUint16(val);           x.getInt8(marker);
            ASSERT(1 == val);           ASSERT('\xFF' == marker);
            x.getUint16(val);           x.getInt8(marker);
            ASSERT(2 == val);           ASSERT('\xFE' == marker);
            x.getUint16(val);           x.getInt8(marker);
            ASSERT(3 == val);           ASSERT('\xFD' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint16(4);             o.putInt8(0xFD);
            o.putUint16(5);             o.putInt8(0xFE);
            o.putUint16(6);             o.putInt8(0xFF);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char marker;
            unsigned short val;
            x.getUint16(val);           x.getInt8(marker);
            ASSERT(4 == val);           ASSERT('\xFD' == marker);
            x.getUint16(val);           x.getInt8(marker);
            ASSERT(5 == val);           ASSERT('\xFE' == marker);
            x.getUint16(val);           x.getInt8(marker);
            ASSERT(6 == val);           ASSERT('\xFF' == marker);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 5: {
        // --------------------------------------------------------------------
        // GET 8-BIT INTEGERS TEST:
        //   Note that getInt8(char&) is tested in the PRIMARY MANIPULATORS
        //   test.
        //
        // Testing:
        //   getInt8(signed char& variable);
        //   getUint8(char& variable);
        //   getUint8(unsigned char& variable);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "GET 8-BIT INTEGERS TEST" << endl
                 << "=======================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getInt8(signed char&)." << endl;
        }
        {
            Out o;
            o.putInt8(1);
            o.putInt8(2);
            o.putInt8(3);
            o.putInt8(4);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            signed char val;
            x.getInt8(val);            ASSERT(1 == val);
            x.getInt8(val);            ASSERT(2 == val);
            x.getInt8(val);            ASSERT(3 == val);
            x.getInt8(val);            ASSERT(4 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putInt8(5);
            o.putInt8(6);
            o.putInt8(7);
            o.putInt8(8);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            signed char val;
            x.getInt8(val);            ASSERT(5 == val);
            x.getInt8(val);            ASSERT(6 == val);
            x.getInt8(val);            ASSERT(7 == val);
            x.getInt8(val);            ASSERT(8 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint8(char&)." << endl;
        }
        {
            Out o;
            o.putUint8(1);
            o.putUint8(2);
            o.putUint8(3);
            o.putUint8(4);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char val;
            x.getUint8(val);            ASSERT(1 == val);
            x.getUint8(val);            ASSERT(2 == val);
            x.getUint8(val);            ASSERT(3 == val);
            x.getUint8(val);            ASSERT(4 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint8(5);
            o.putUint8(6);
            o.putUint8(7);
            o.putUint8(8);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            char val;
            x.getUint8(val);            ASSERT(5 == val);
            x.getUint8(val);            ASSERT(6 == val);
            x.getUint8(val);            ASSERT(7 == val);
            x.getUint8(val);            ASSERT(8 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting getUint8(unsigned char&)." << endl;
        }
        {
            Out o;
            o.putUint8(1);
            o.putUint8(2);
            o.putUint8(3);
            o.putUint8(4);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            unsigned char val;
            x.getUint8(val);            ASSERT(1 == val);
            x.getUint8(val);            ASSERT(2 == val);
            x.getUint8(val);            ASSERT(3 == val);
            x.getUint8(val);            ASSERT(4 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Out o;
            o.putUint8(5);
            o.putUint8(6);
            o.putUint8(7);
            o.putUint8(8);

            Obj x(o.data(), o.length());
            if (veryVerbose) { P(x) }
            unsigned char val;
            x.getUint8(val);            ASSERT(5 == val);
            x.getUint8(val);            ASSERT(6 == val);
            x.getUint8(val);            ASSERT(7 == val);
            x.getUint8(val);            ASSERT(8 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
      } break;
      case 4: {
        // --------------------------------------------------------------------
        // PRINT OPERATOR TEST:
        //   For each of a small representative set of objects, use
        //   'ostrstream' to write that object's value to a string buffer and
        //   then compare this buffer with the expected output format.
        //
        // Testing:
        //   ostream& operator<<(ostream&, const ByteInStream&);
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "PRINT OPERATOR TEST" << endl
                 << "===================" << endl;
        }

        if (verbose) {
            cout << "\nTesting print operator." << endl;
        }

        const int SIZE = 1000;     // Must be big enough to hold output string.
        const char XX = (char) 0xFF; // Value that represents an unset char.
        char ctrl[SIZE];    memset(ctrl, XX, SIZE);
        const char *CTRL = ctrl;

        {
            Obj x;
            const char *EXPECTED = "";
            char buf[SIZE];
            memcpy(buf, CTRL, SIZE);
            ostrstream out(buf, SIZE);    out << x << ends;
            const int LEN = strlen(EXPECTED) + 1;
            if (veryVerbose) cout << "\tEXPECTED : " << EXPECTED << endl
                                  << "\tACTUAL : "   << buf << endl;
            ASSERT(XX == buf[SIZE-1]); // check for overrun
            ASSERT(0 == memcmp(buf, EXPECTED, LEN));
            ASSERT(0 == memcmp(buf + LEN, CTRL + LEN, SIZE - LEN));
        }
        {
            Out o;  o.putInt8(0);  o.putInt8(1);  o.putInt8(2);  o.putInt8(3);
            Obj x(o.data(), o.length());
            const char *EXPECTED =
                "\n0000\t00000000 00000001 00000010 00000011";
            char buf[SIZE];
            memcpy(buf, CTRL, SIZE);
            ostrstream out(buf, SIZE);    out << x << ends;
            const int LEN = strlen(EXPECTED) + 1;
            if (veryVerbose) cout << "\tEXPECTED : " << EXPECTED << endl
                                  << "\tACTUAL : "   << buf << endl;
            ASSERT(XX == buf[SIZE-1]); // check for overrun
            ASSERT(0 == memcmp(buf, EXPECTED, LEN));
            ASSERT(0 == memcmp(buf + LEN, CTRL + LEN, SIZE - LEN));
        }
        {
            Out o;
            o.putInt8(0);  o.putInt8(1);  o.putInt8(2);  o.putInt8(3);
            o.putInt8(4);  o.putInt8(5);  o.putInt8(6);  o.putInt8(7);
            o.putInt8(8);  o.putInt8(9);  o.putInt8(10); o.putInt8(11);
            Obj x(o.data(), o.length());
            const char *EXPECTED =
                "\n0000\t00000000 00000001 00000010 00000011 "
                        "00000100 00000101 00000110 00000111"
                "\n0008\t00001000 00001001 00001010 00001011";
            char buf[SIZE];
            memcpy(buf, CTRL, SIZE);
            ostrstream out(buf, SIZE);    out << x << ends;
            const int LEN = strlen(EXPECTED) + 1;
            if (veryVerbose) cout << "\tEXPECTED : " << EXPECTED << endl
                                  << "\tACTUAL : "   << buf << endl;
            ASSERT(XX == buf[SIZE-1]); // check for overrun
            ASSERT(0 == memcmp(buf, EXPECTED, LEN));
            ASSERT(0 == memcmp(buf + LEN, CTRL + LEN, SIZE - LEN));
        }
      } break;
      case 3: {
        // --------------------------------------------------------------------
        // BASIC ACCESSORS TEST:
        //   For each independent test, create objects containing various data.
        //   Use basic accessors to verify the initial state of the objects.
        //   Then use primary manipulators 'getInt8' and 'invalidate' to modify
        //   the state of the objects, and use basic accessors to verify the
        //   resulting state.
        //
        // Testing
        //   operator const void *() const;
        //   bool isValid() const;
        //   const char *data() const;
        //   bool isEmpty() const;
        //   int length() const;
        //   int cursor() const;
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "BASIC ACCESSORS TEST" << endl
                 << "====================" << endl;
        }
        if (verbose) {
            cout << "\nTesting operator const void *(), isValid() and data()."
                 << endl;
        }
        int i, j;
        for (i = 0; i < 5; i++) {
            Obj x;
            LOOP_ASSERT(i, x);
            LOOP_ASSERT(i, 0 == x.data());

            Out o;
            for (j = 0; j < i;  j++) o.putInt8(j);

            Obj x2(o.data(), o.length());
            if (veryVerbose) { P(x2) }
            LOOP_ASSERT(i, x2 && x2.isValid());
            LOOP_ASSERT(i, 0 == bsl::memcmp(x2.data(), o.data(), o.length()));

            LOOP_ASSERT(i, x && x2);
            LOOP_ASSERT(i, x.isValid() && x2.isValid());
            x.invalidate();
            LOOP_ASSERT(i, !x && x2);
            LOOP_ASSERT(i, !x.isValid() && x2.isValid());

            // invalidate stream x2 by making excessive 'get' calls
            char c;
            for (j = 0; j < i + 1; j++) x2.getInt8(c);
            LOOP_ASSERT(i, !x && !x2);
            LOOP_ASSERT(i, !x.isValid() && !x2.isValid());
        }

        // --------------------------------------------------------------------

        if (verbose)
            cout << "\nTesting isEmpty(), length() and cursor()." << endl;

        for (i = 0; i < 5; i++) {
            // test default empty objects
            Obj x;
            LOOP_ASSERT(i, x.isEmpty());
            LOOP_ASSERT(i, x.length() == 0);
            LOOP_ASSERT(i, x.cursor() == 0);

            // test objects of variable lengths
            Out o;
            for (j = 0; j < i; j++) {
                o.putInt8(j);
            }

            Obj x2(o.data(), o.length());
            if (veryVerbose) { P(x2) }
            LOOP_ASSERT(i, (0 == i && x2.isEmpty()) || !x2.isEmpty());
            LOOP_ASSERT(i, x2.length() == i);
            LOOP_ASSERT(i, x2.cursor() == 0);

            char c;
            for (j = 0; j < i; j++) {
                LOOP2_ASSERT(i,
                             j,
                             x2.cursor() == SIZEOF_INT8 * j);
                x2.getInt8(c);
            }
            LOOP_ASSERT(i, x2.isEmpty());
        }
      } break;
      case 2: {
        // --------------------------------------------------------------------
        // PRIMARY MANIPULATORS TEST:
        //   For each independent test, create objects containing various data,
        //   then use primary manipulators 'getInt8' and 'invalidate' to modify
        //   the state of the objects.  Use basic accessors to verify the final
        //   state of the objects.
        //
        // Testing
        //   ByteInStream();
        //   ByteInStream(const char *buffer, int numBytes);
        //   ~ByteInStream();   // by purify
        //   getInt8(char& variable);
        //   void invalidate();
        // --------------------------------------------------------------------

        if (verbose) {
            cout << endl
                 << "PRIMARY MANIPULATORS TEST" << endl
                 << "=========================" << endl;
        }

        if (verbose) {
            cout << "\nTesting getInt8(char&) and ctors." << endl;

        }
        {
            Obj x0; // test default ctor
            if (veryVerbose) { P(x0) }
            ASSERT(x0);
            ASSERT(0 == x0.length());

            Out o;
            o.putInt8(1);
            o.putInt8(2);
            o.putInt8(3);
            o.putInt8(4);

            Obj x(o.data(), o.length());
                                             // test ctor initialized w/ char *
            ASSERT(x);
            ASSERT(x.length() == o.length());
            if (veryVerbose) { P(x) }
            char val;
            x.getInt8(val);              ASSERT(1 == val);
            x.getInt8(val);              ASSERT(2 == val);
            x.getInt8(val);              ASSERT(3 == val);
            x.getInt8(val);              ASSERT(4 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }
        {
            Obj x0;
            if (veryVerbose) { P(x0) }
            ASSERT(x0);
            ASSERT(0 == x0.length());

            Out o;
            o.putInt8(5);
            o.putInt8(6);
            o.putInt8(7);
            o.putInt8(8);

            Obj x(o.data(), o.length());
            ASSERT(x);
            ASSERT(x.length() == o.length());
            if (veryVerbose) { P(x) }
            char val;
            x.getInt8(val);              ASSERT(5 == val);
            x.getInt8(val);              ASSERT(6 == val);
            x.getInt8(val);              ASSERT(7 == val);
            x.getInt8(val);              ASSERT(8 == val);
            ASSERT(x);
            ASSERT(x.isEmpty());
            ASSERT(x.cursor() == x.length());
        }

        // --------------------------------------------------------------------

        if (verbose) {
            cout << "\nTesting invalidate()." << endl;
        }
        for (int i = 0; i < 5; i++) {
            // test default objects
            Obj x;
            LOOP_ASSERT(i, x);
            x.invalidate();
            LOOP_ASSERT(i, !x);

            // test objects of variable lengths
            Out o;
            for (int j = 0; j < i;  j++) o.putInt8(j);

            Obj x2(o.data(), o.length());
            if (veryVerbose) { P(x2) }
            LOOP_ASSERT(i, x2);
            x2.invalidate();
            LOOP_ASSERT(i, !x2);
        }
      } break;
      case 1: {
        // --------------------------------------------------------------------
        // BREATHING TEST:
        //   Create 'ByteInStream' objects containing various data.
        //   Exercise these objects using appropriate input methods and primary
        //   manipulators, then verify the state of the resulting objects using
        //   basic accessors.
        //
        // Testing:
        //   This "test" exercises basic functionality, but tests nothing.
        // --------------------------------------------------------------------

        if (verbose) {
          cout << endl
               << "BREATHING TEST" << endl
               << "==============" << endl;
        }
        if (verbose) {
            cout << "\nCreate object x1 using default ctor." << endl;
        }

        int i;
        Obj x1;
        ASSERT(0 == x1.length());

        if (verbose) {
            cout << "\nCreate object x2 w/ an initial value." << endl;
        }
        Obj x2("\x00\x01\x02\x03\x04", 5);
        if (veryVerbose) { P(x2); }
        ASSERT(5 == x2.length());

        const char *data = x2.data();

        if (verbose) {
            cout << "\nTry getInt8() with x2." << endl;
        }
        for (i = 0; i < 5; i++) {
            char c;
            x2.getInt8(c);
            LOOP_ASSERT(i, i == c);
        }

        if (verbose) {
            cout << "\nTry isEmpty() with x2." << endl;
        }
        ASSERT(1 == x2.isEmpty());

        if (verbose) {
            cout << "\nTry invalidate() with x2." << endl;
        }
        x2.invalidate();
        ASSERT(!x2);

        if (verbose) {
            cout << "\nTry invalid operation with x1." << endl;
        }
        x1.getInt32(i);
        ASSERT(!x1);
      } break;
      default: {
        cerr << "WARNING: CASE `" << test << "' NOT FOUND." << endl;
        testStatus = -1;
      }
    }

    if (testStatus > 0) {
        cerr << "Error, non-zero test status = " << testStatus << "." << endl;
    }
    return testStatus;
}

// ---------------------------------------------------------------------------
// NOTICE:
// Copyright (c) 2013. Bloomberg Finance L.P.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ----------------------------- END-OF-FILE ---------------------------------