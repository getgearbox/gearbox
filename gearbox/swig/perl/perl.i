%perlcode %{
  # needed for JSON::true, JSON::false in fromJson() used in %typemap(out)
  use JSON;
%}

%{
  #include <gearbox/core/logger.h>

  void typeDump(void*vv) {
    SV * sv = (SV*)vv;
    if( SvIOK(sv) ) _DEBUG("SvIOK: " << SvIOK(sv));
    if( SvIOKp(sv) ) _DEBUG("SvIOKp: " << SvIOKp(sv));
    if( SvIOK_notUV(sv) ) _DEBUG("SvIOK_notUV: " << SvIOK_notUV(sv));
    if( SvIOK_UV(sv) ) _DEBUG("SvIOK_UV: " << SvIOK_UV(sv));
    if( SvNIOK(sv) ) _DEBUG("SvNIOK: " << SvNIOK(sv));
    if( SvNIOKp(sv) ) _DEBUG("SvNIOKp: " << SvNIOKp(sv));
    if( SvNOK(sv) ) _DEBUG("SvNOK: " << SvNOK(sv));
    if( SvNOKp(sv) ) _DEBUG("SvNOKp: " << SvNOKp(sv));
    if( SvOK(sv) ) _DEBUG("SvOK: " << SvOK(sv));
    if( SvOOK(sv) ) _DEBUG("SvOOK: " << SvOOK(sv));
    if( SvPOK(sv) ) _DEBUG("SvPOK: " << SvPOK(sv));
    if( SvPOKp(sv) ) _DEBUG("SvPOKp: " << SvPOKp(sv));
    if( SvROK(sv) ) _DEBUG("SvROK: " << SvROK(sv));
    if( SvUOK(sv) ) _DEBUG("SvUOK: " << SvUOK(sv));
    if( sv_isobject(sv) ) {
      _DEBUG("IsOBJ: " << sv_isobject(sv));
      _DEBUG("CLASSNAME: " << HvNAME(SvSTASH(SvRV(sv))));
    }
    if( (SvTYPE(sv) == SVt_NULL) ) _DEBUG("SVt_NULL: " << (SvTYPE(sv) == SVt_NULL));
    if( (SvTYPE(sv) == SVt_IV) ) _DEBUG("SVt_IV: " << (SvTYPE(sv) == SVt_IV));
    if( (SvTYPE(sv) == SVt_NV) ) _DEBUG("SVt_NV: " << (SvTYPE(sv) == SVt_NV));
    if( (SvTYPE(sv) == SVt_RV) ) _DEBUG("SVt_RV: " << (SvTYPE(sv) == SVt_RV));
    if( (SvTYPE(sv) == SVt_PV) ) _DEBUG("SVt_PV: " << (SvTYPE(sv) == SVt_PV));
    if( (SvTYPE(sv) == SVt_PVIV) ) _DEBUG("SVt_PVIV: " << (SvTYPE(sv) == SVt_PVIV));
    if( (SvTYPE(sv) == SVt_PVNV) ) _DEBUG("SVt_PVNV: " << (SvTYPE(sv) == SVt_PVNV));
    if( (SvTYPE(sv) == SVt_PVMG) ) _DEBUG("SVt_PVMG: " << (SvTYPE(sv) == SVt_PVMG));
    if( (SvTYPE(sv) == SVt_PVBM) ) _DEBUG("SVt_PVBM: " << (SvTYPE(sv) == SVt_PVBM));
    if( (SvTYPE(sv) == SVt_PVLV) ) _DEBUG("SVt_PVLV: " << (SvTYPE(sv) == SVt_PVLV));
    if( (SvTYPE(sv) == SVt_PVAV) ) _DEBUG("SVt_PVAV: " << (SvTYPE(sv) == SVt_PVAV));
    if( (SvTYPE(sv) == SVt_PVHV) ) _DEBUG("SVt_PVHV: " << (SvTYPE(sv) == SVt_PVHV));
    if( (SvTYPE(sv) == SVt_PVCV) ) _DEBUG("SVt_PVCV: " << (SvTYPE(sv) == SVt_PVCV));
    if( (SvTYPE(sv) == SVt_PVGV) ) _DEBUG("SVt_PVGV: " << (SvTYPE(sv) == SVt_PVGV));
    if( (SvTYPE(sv) == SVt_PVFM) ) _DEBUG("SVt_PVFM: " << (SvTYPE(sv) == SVt_PVFM));
    if( (SvTYPE(sv) == SVt_PVIO) ) _DEBUG("SVt_PVIO: " << (SvTYPE(sv) == SVt_PVIO));
    _DEBUG("REFCOUNT: " << SvREFCNT(sv));
  }

  static void toJson(Json & json, SV * sv);

  static void toJsonString(Json & json, SV * sv) {
      STRLEN len; 
      char * ptr = SvPV(sv,len);
      json = std::string(ptr,len);
      return;
  }

  static void toJsonBool(Json & json, SV * sv) {
    // only called if sv is JSON::Boolean object
    // it is just a blessed int scalar ref 
    // so deref and get the IV for it
    int val = SvIV(SvRV(sv));
    json = val ? true : false;
    return;
  }

  static void toJsonObject(Json & json, HV * hv) {
    json = Json::Object();
    if( hv_iterinit(hv) ) {
      SV * val;
      I32 klen;
      char *key;
      while( NULL != (val = hv_iternextsv(hv,&key,&klen)) ) {
        toJson(json[std::string(key,klen)], val);
      }
    }
    return;
  }
  
  static void toJsonArray(Json & json, AV * av) {
    int len = av_len(av);
    json = Json::Array();
    for( int i=0; i <= len; i++ ) {
      SV ** sv = av_fetch(av, i, 0);
      toJson(json[i], *sv);
    }
    return;
  }

  static void toJson(Json & json, SV * sv) {
    switch(SvTYPE(sv)) {
    case SVt_NULL:
      json.empty();
      return;
    case SVt_IV:
      // this will probably be JSON::Boolean class
      if( sv_isobject(sv) ) {
        if( sv_derived_from(sv, "JSON::Boolean") ) {
          toJsonBool(json,sv);
          return;
        }
      }
      // if it is an object we will just ignore the class
      // and literaly deserialize the blessed reference
      // as a data-structure
      return toJson(json, SvRV(sv));
    case SVt_PVIV:
      json = static_cast<int64_t>(SvIV(sv));
      return;
    case SVt_NV:
    case SVt_PVNV:
      json = static_cast<double>(SvNV(sv));
      return;
    case SVt_PV:
      toJsonString(json,sv);
      return;
    case SVt_PVHV:
      toJsonObject(json,(HV*)sv);
      return;
    case SVt_PVAV:
      toJsonArray(json,(AV*)sv);
      return;
    }
  }

  static SV * fromJson(Json & json) {
    switch( json.type() ) {
    case Json::UNDEF:
      return newSV(0);
    case Json::BOOL:
      return json.as<bool>() ? SvREFCNT_inc(get_sv("JSON::true",0)) : SvREFCNT_inc(get_sv("JSON::false",0));
    case Json::INT:
      return newSViv(json.as<int64_t>());
    case Json::DOUBLE:
      return newSVnv(json.as<double>());
    case Json::STRING: {
      std::string & str = json.as<std::string>();
      return newSVpv(str.c_str(), str.size());
    }
    case Json::OBJECT: {
      HV * hv = newHV();
      Json::Object & obj = json.as<Json::Object>();
      Json::Object::iterator i = obj.begin();
      Json::Object::iterator e = obj.end();
      for( ; i != e; ++i ) {
        hv_store(hv,i->first.c_str(),i->first.size(), fromJson(*(i->second)), 0);
      }
      return newRV_noinc((SV*)hv);
    }
    case Json::ARRAY:
      AV * av = newAV();
      for( int i=0; i < json.length(); ++i ) {
        av_store(av,i,fromJson(json[i]));
      }
      return newRV_noinc((SV*)av);
    }
  }

  std::string testJson(SV * sv, bool pretty=0) {
    Json json;
    toJson(json,sv);
    return json.serialize(pretty);
  }
%}

// convert Perl native array to std::vector<std::string>
%typemap(in) const std::vector<std::string> & (int len, Array temp) %{
  $1 = &temp;
  len = av_len((AV*)$input);
  for( int i=0; i <= len; i++ ) {
    SV ** sv = av_fetch((AV*)$input, i, 0);
    STRLEN sl;
    char * str = SvPV(*sv,sl);
    $1->push_back(std::string(str,sl));
  }
%}

// convert std::vector<std::string> to Perl native array
%typemap(out) const std::vector<std::string> & {
  AV * av = newAV();
  $result = sv_2mortal(newRV_noinc((SV*)av));
  argvi++;
  Array::const_iterator i = $1->begin();
  Array::const_iterator e = $1->end();
  for(; i != e; ++i ) {
    av_push(av,newSVpv(i->c_str(), i->size()));
  }
}

%typemap(out) const JobPtr & {
  $result = $1->get() ? SWIG_NewPointerObj(SWIG_as_voidptr($1->get()), SWIGTYPE_p_Gearbox__Job, SWIG_SHADOW) : newSV(0);
  argvi++;
}

%typemap(out) JobPtr {
  $result = $1.get() ? SWIG_NewPointerObj(SWIG_as_voidptr(new Job(*($1.get()))), SWIGTYPE_p_Gearbox__Job, SWIG_OWNER|SWIG_SHADOW) : newSV(0);
  argvi++;
}

%typemap(out) Gearbox::JobPtr {
  $result = $1.get() ? SWIG_NewPointerObj(SWIG_as_voidptr(new Job(*($1.get()))), SWIGTYPE_p_Gearbox__Job, SWIG_OWNER|SWIG_SHADOW) : newSV(0);
  argvi++;
}

%typemap(out) const StatusPtr & {
  $result = $1->get() ? SWIG_NewPointerObj(SWIG_as_voidptr($1->get()), SWIGTYPE_p_Gearbox__Status, SWIG_SHADOW) : newSV(0);
  argvi++;
}

%typemap(out) Gearbox::StatusPtr {
  $result = $1.get() ? SWIG_NewPointerObj(SWIG_as_voidptr(new Status(*($1.get()))), SWIGTYPE_p_Gearbox__Status, SWIG_OWNER|SWIG_SHADOW) : newSV(0);
  argvi++;
}

%typemap(out) Gearbox::JobQueue {
  AV * levels = newAV();
  $result = sv_2mortal(newRV_noinc((SV*)levels));
  argvi++;
  Gearbox::JobQueue::const_iterator i = $1.begin();
  Gearbox::JobQueue::const_iterator e = $1.end();
  for(; i != e; ++i ) {
    AV * level = newAV();
    av_push(levels,newRV_noinc((SV*)level));
    std::vector<JobPtr>::const_iterator li = i->begin();
    std::vector<JobPtr>::const_iterator le = i->end();
    for( ; li != le; ++li ) {
      SV * job = newSV(0);
      if( li->get() ) {
        Job * j = new Job( *(li->get()) );
        SWIG_MakePtr(job, SWIG_as_voidptr(j), SWIGTYPE_p_Gearbox__Job, SWIG_OWNER|SWIG_SHADOW);
      }
      av_push(level,job);
    }
  }
}

%typemap(in) Gearbox::JobQueue & (JobQueue temp) {
  {
    AV * av = (AV*)SvRV($input);
    $1 = &temp;
    int len = av_len(av);
    for( int i=0; i <= len; i++ ) {
      SV ** levelsv = av_fetch(av,i,0);
      AV * level = (AV*)SvRV(*levelsv);
      int level_len = av_len(level);
      temp.push_back(std::vector<JobPtr>());
      for( int j=0; j <= level_len; j++ ) {
        SV ** job = av_fetch(level,j,0);
        Job * jptr;
        SWIG_ConvertPtr(*job,(void**)&jptr,SWIGTYPE_p_Gearbox__Job,0);
        temp[i].push_back(boost::shared_ptr<Job>(new Job(*jptr)));
      }
    }
  }
}

%typemap(typecheck) const Json & %{
  $1 = 1;
%}

// For functions that take a Hash ... accept a SV* from PERL
// and construct the Hash object on the fly
%typemap(typecheck) const Hash & %{
  $1 = ( SvTYPE(SvRV($input)) == SVt_PVHV ) ? 1 : 0;
%}

// convert Perl SV* hash ref into Hash c++ object
%typemap(in) const Hash & (HV * hv, Hash temp) %{
  hv = (HV*)SvRV($input);
  $1 = &temp;
  if( hv_iterinit(hv) ) {
    SV * val;
    I32 klen;
    char *key;
    while( NULL != (val = hv_iternextsv(hv,&key,&klen)) ) {
      STRLEN len;
      char * ptr = SvPV(val,len);
      temp[std::string(key,klen)] = std::string(ptr,len);
    }
  }
%}

// For functions that take an Array ... accept a SV* from PERL
// and construct the atray object on the fly
%typemap(typecheck) const std::vector<std::string> & %{
  $1 = ( SvTYPE(SvRV($input)) == SVt_PVAV ) ? 1 : 0;
%}

// convert Perl SV* hash ref into Hash c++ object
%typemap(in) const std::vector<std::string> & (AV * av, std::vector<std::string> temp) %{
  {
    av = (AV*)SvRV($input);
    $1 = &temp;
    int len = av_len(av);
    for( int i=0; i <= len; i++ ) {
      SV ** sv = av_fetch(av, i, 0);
      STRLEN svlen;
      char * ptr = SvPV(*sv,svlen);
      temp.push_back(std::string(ptr,svlen));
    }
  }
%}

// for PERL make Hash response into perl HV RV
%typemap(out) const Hash & {
  HV * hv = newHV();
  $result = sv_2mortal(newRV_noinc((SV*)hv));
  argvi++;
  Hash::iterator i = $1->begin();
  Hash::iterator e = $1->end();
  for( ; i != e; ++i ) {
    SV * val = newSVpv(i->second.c_str(), i->second.size());
    hv_store(hv,i->first.c_str(),i->first.size(), val, 0);
  }
}

// For functions that take a ConfigFile ... accept a string from Perl
// and construct the ConfigFile object on the fly
%typemap(typecheck) const ConfigFile & %{
  $1 = SvPOKp($input) ? 1 : 0;
%}

%typemap(in) const ConfigFile & (std::string filename, STRLEN len, std::auto_ptr<ConfigFile> cfptr) %{
  {
    char * ptr = SvPV($input,len);
    filename.assign(ptr, len);
    cfptr.reset(new ConfigFile(filename));
    $1 = cfptr.get();
  }
%}

// For functions that take a Uri ... accept a string from Perl
// and construct the Uri object on the fly
%typemap(typecheck) const Uri & %{
  $1 = SvPOKp($input) ? 1 : 0;
%}

%typemap(in) const Uri & (std::string uri, STRLEN len, std::auto_ptr<Uri> cfptr) %{
  {
    char * ptr = SvPV($input,len);
    uri.assign(ptr, len);
    cfptr.reset(new Uri(uri));
    $1 = cfptr.get();
  }
%}

%typemap(in) const Json & (Json temp) %{
  $1 = &temp;
  toJson(temp,$input);
%}

%typemap(out) const Json & (std::string temp) %{
  $result = sv_2mortal( fromJson(*$1) );
  argvi++;
%}

%exception {
  try { $action  }
  catch(const Error & err) {
    // reset $@ to be new blessed scalar ref object of type
    // ERR_$name
    std::string pkg("ERR_");
    pkg += err.name();
    HV * excepthv = newHV();
    hv_store(excepthv,"msg", 3, newSVpv(err.what(), strlen(err.what())),0);
    hv_store(excepthv,"name", 4, newSVpv(err.name(), strlen(err.name())),0);
    hv_store(excepthv, "code", 4, newSViv(err.code()),0);
    SV * errsv = get_sv("@", TRUE);
    sv_setsv(errsv,newSVrv((SV*)excepthv,pkg.c_str()));
    croak(Nullch);
  }
  catch(const std::exception & err) {
    croak(err.what());
  }
}

std::string testJson(SV * sv, bool pretty=0);

