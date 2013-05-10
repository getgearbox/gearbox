#ifndef GEARBOX_JSON_H
#define GEARBOX_JSON_H
#include "config.h"
#include <map>
#include <vector>
#include <stdexcept>
#include <stdint.h>
#include <boost/shared_ptr.hpp>

namespace Gearbox {

class JsonError : public std::exception {
public:
  JsonError(const std::string ctx, const std::string msg);
  JsonError(const std::string msg);
  ~JsonError() throw ();
  const char * what() const throw ();

private:
    char buffer[512];
};

// this is specialized below after the
// class Json definition
template<typename T> struct TypeToJsonType;

class JsonSchema; // forward declare

class Json {
public:
    typedef std::map< std::string, boost::shared_ptr<Json> > Object;
    typedef std::vector< boost::shared_ptr<Json> > Array;
    typedef bool (*ErrorHandler)(const std::string & error, const std::string & context);
    enum Type {
        // cant use NULL here .. it is globally defined
        UNDEF = 0,
        BOOL,
        INT,
        DOUBLE,
        STRING,
        OBJECT,
        ARRAY
    };

    Json();
    Json(const Json &);

    Json(bool value);
    Json(int value);
    Json(int64_t value);
    Json(double value);
    Json(const char * value);
    Json(const std::string & value);
    Json(const Object & value);
    Json(const Array & value);

    ~Json();
    
    Json & parse(const std::string & content);
    Json & parseFile(const std::string & file);
    bool hasKey(const std::string & key) const;

    bool operator==(const Json &other) const;
    bool operator!=(const Json &other) const;

    const Json & operator[](const std::string & key) const;
    const Json & operator[](int i) const;

    const Json & get(const std::string & key) const;
    const Json & at(int i) const;

    const Json * parent() const;
    Json * parent();
    
    void setSchema(JsonSchema * schema);
    bool validate(JsonSchema * schema);

    Json & operator[](const std::string & key);
    Json & operator[](int i);

    Json & get(const std::string & key);
    Json & at(int i);

    Json::Type type() const;
    std::string typeName() const;

    // fetch for types specialized below (const)
    template<typename T> 
    typename TypeToJsonType<T>::RetType as() const {
        this->assertType(TypeToJsonType<T>::Value);
        return *static_cast<typename TypeToJsonType<T>::RealType*>(this->data_);
    }

    // fetch for types specialized below
    template<typename T> 
    typename TypeToJsonType<T>::RetType as() {
        this->assertType(TypeToJsonType<T>::Value);
        return *static_cast<typename TypeToJsonType<T>::RealType*>(this->data_);
    }

    // fetch for types specialized below
    template<typename T>
    void to(T & data) const {
        this->assertType(TypeToJsonType<T>::Value);
        data = *(static_cast<typename TypeToJsonType<T>::RealType*>(this->data_));
    }

    Json & operator=(const Json & json);
    Json & operator=(char * value);
    Json & operator=(const char * value);

    // setters for types specialized below
    template<typename T>
    Json & operator=(const T & value) {
        // dont free data if assigning to ourselves
        // ie: json["key"] = json["key"].as<std::string>() 
        if( data_ == &value ) return *this;;
        this->setData(new typename TypeToJsonType<T>::RealType(value), TypeToJsonType<T>::Value);
        return *this;
    }

    // this init routine will let you initialize a json object
    // to the type you want and the reference to the primitive
    // object will be returned.
    template<typename T>
    typename TypeToJsonType<T>::RealType & init() {
        typedef typename TypeToJsonType<T>::RealType PrimitiveType;
        PrimitiveType * d = new PrimitiveType();
        this->setData(d, TypeToJsonType<T>::Value);
        return *d;
   }
        
    // return length of array type, -1 otherwise
    int length() const;

    // clear contents, reset to null
    void clear();

    // if container type (array, object), is the container empty?
    bool empty() const;

    bool deleteKey(const std::string & key);
    
    std::string serialize(bool beautify = false) const;

    void setErrorHandler(ErrorHandler err);
    std::string errorMessage() const;
    
    // error callbacks
    static bool onErrorThrow(const std::string & error, const std::string & context);
    static bool onErrorWarn(const std::string & error, const std::string & context);
    
    std::string context() const;
    
    class Parser; // forward declare
    class Generator; // forward declare
    friend class Parser;
    friend class Generator;

protected:
    void freeData();
    bool equals( const Json & other ) const;
    bool equals( const Json::Object & o ) const;
    bool equals( const Json::Array & a ) const;
    void   copy(const Json & other);
    void * copy(const Json::Object & o);
    void * copy(const Json::Array  & a);
    void setData(void *, int);
    static std::string typeToName(int type);
    void assertType(int type) const;
    Json & addKey(const std::string & key);
    void growArray(int size);
    bool validateObject(JsonSchema * schema);
    bool validateArray(JsonSchema * schema);
    bool handleError(const std::string & error, const std::string & context) const;
    class Private;
    Private * impl_;
    void * data_;
};

template<> struct TypeToJsonType<bool> { 
    enum JsonType { Value = Json::BOOL }; 
    typedef bool RealType;
    typedef RealType & RetType;
};

template<> struct TypeToJsonType<int>  {
    enum JsonType { Value = Json::INT };
    typedef int64_t RealType;
    typedef int RetType;
};

template<> struct TypeToJsonType<long> {
    enum JsonType { Value = Json::INT };
    typedef int64_t RealType;
# if __WORDSIZE == 64 && SIZEOF_LONG == 8
    typedef RealType & RetType;
# else
    typedef long RetType;
# endif
};

template<> struct TypeToJsonType<unsigned int>  {
    enum JsonType { Value = Json::INT };
    typedef int64_t RealType;
    typedef unsigned int RetType;
};

template<> struct TypeToJsonType<unsigned long> {
    enum JsonType { Value = Json::INT };
    typedef int64_t RealType;
# if __WORDSIZE == 64 && SIZEOF_LONG == 8
    typedef RealType & RetType;
# else
    typedef unsigned long RetType;
# endif
};

#ifdef HAVE_LONG_LONG_INT
template<> struct TypeToJsonType<long long>  {
    enum JsonType { Value = Json::INT };
    typedef int64_t RealType;
# if __WORDSIZE == 64 && SIZEOF_LONG_LONG == 8
    typedef RealType & RetType;
# else
    typedef long long RetType;
# endif
};
#endif

#ifdef HAVE_UNSIGNED_LONG_LONG_INT
template<> struct TypeToJsonType<unsigned long long>  {
    enum JsonType { Value = Json::INT };
    typedef int64_t RealType;
# if __WORDSIZE == 64 && SIZEOF_LONG_LONG == 8
    typedef RealType & RetType;
# else
    typedef unsigned long long RetType;
# endif    
};
#endif

template<> struct TypeToJsonType<float>  {
    enum JsonType { Value = Json::DOUBLE };
    typedef double RealType;
    typedef float RetType;
};

template<> struct TypeToJsonType<double> {
    enum JsonType { Value = Json::DOUBLE };
    typedef double RealType;
    typedef RealType & RetType;
};

template<> struct TypeToJsonType<std::string>  {
    enum JsonType { Value = Json::STRING };
    typedef std::string RealType;
    typedef RealType & RetType;
};

template<> struct TypeToJsonType<Json::Object> {
    enum JsonType { Value = Json::OBJECT };
    typedef Json::Object RealType;
    typedef RealType & RetType;
};

template<> struct TypeToJsonType<Json::Array>  {
    enum JsonType { Value = Json::ARRAY };
    typedef Json::Array RealType;
    typedef RealType & RetType;
};

std::ostream & operator<<(std::ostream & os, const Json & json);

} // namespace

#endif // GEARBOX_JSON_H
