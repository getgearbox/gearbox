%{
  class PerlWorker : public Gearbox::Worker {
     typedef Gearbox::Worker super;
   public:
     PerlWorker(const std::string &config) : Worker(config), self(NULL) {};
     virtual ~PerlWorker() {
       if ( self ) {
         SvREFCNT_dec( self );
       }  
     }
     void register_handler( const std::string & function_name, SV * func=NULL) {
       if( func ) {
         funcs[function_name] = newSVsv(func);
       }
       else {
         std::string package = HvNAME(SvSTASH(self));
         std::string fname = package + "::" + function_name;
         funcs[function_name] = newSVpv(fname.c_str(), fname.size());
       }
       this->Worker::register_handler(
         function_name,
          static_cast<Worker::handler_t>(&PerlWorker::redispatcher)
       );
     }

     int clear_stack(int stack_count) {
       dSP;
       int result = 0;
       SV * errsv = get_sv("@", TRUE);
       if( stack_count ) {
         SPAGAIN;
         if( SvTRUE(errsv) ) {
           // pop undef off stack when we get an eval error
           POPs;
         } else {
           result = POPi;
         }
         PUTBACK;
       }
       FREETMPS;
       LEAVE;

       if( SvTRUE(errsv) ) {
         if( sv_isobject(errsv) && sv_derived_from(errsv, "Gearbox::Error") ) {
           HV*self=(HV*)SvRV(errsv);
           STRLEN len;
           char * errmsgptr = SvPV(*hv_fetch(self,"msg",3,0), len);
           throw_from_code(
             SvIV(*hv_fetch(self,"code",4,0)),
             std::string(errmsgptr, len)
            );
         }
         else {
           STRLEN len;
           gbTHROW(std::runtime_error(std::string(SvPV(errsv,len),len)));
         }                     
       }
       return result;
     }

     void request_wrapper(const Job & job, const std::string & phase) {
       std::string fname = phase+ "_request";
       dSP;

       SV * j = SWIG_NewPointerObj(SWIG_as_voidptr(&job), SWIGTYPE_p_Gearbox__Job, SWIG_SHADOW);
       ENTER;
       SAVETMPS;
       PUSHMARK(SP);
       EXTEND(SP,2);
       PUSHs(sv_2mortal(newRV_inc((SV*)self)));
       PUSHs(j);
       PUTBACK;
       int stack_count = call_method(fname.c_str(), G_SCALAR|G_EVAL);
       clear_stack(stack_count);
     }

     void pre_request(const Job & job) { 
       this->request_wrapper(job,"pre");
       this->super::pre_request(job);
     }
     
     void post_request(const Job & job) { 
       this->request_wrapper(job,"post");
       this->super::post_request(job);
     }

     Worker::response_t redispatcher(const Job & job, JobResponse & resp) {
       SV * j = SWIG_NewPointerObj(SWIG_as_voidptr(&job), SWIGTYPE_p_Gearbox__Job, SWIG_SHADOW);
       SV * r = SWIG_NewPointerObj(SWIG_as_voidptr(&resp), SWIGTYPE_p_Gearbox__JobResponse, SWIG_SHADOW);
       dSP;
       ENTER;
       SAVETMPS;
       PUSHMARK(SP);
       EXTEND(SP,3);
       PUSHs(sv_2mortal(newRV_inc((SV*)self)));
       PUSHs(j);
       PUSHs(r);
       PUTBACK;
       int stack_count = call_sv(this->funcs[job.name()], G_SCALAR|G_EVAL);
       int result = this->clear_stack(stack_count);
       return static_cast<Worker::response_t>(result);
     }
     void set_self(SV* s) {
       self = SvREFCNT_inc( SvRV(s) );
     }
       
   private:
     std::map<std::string,SV*> funcs;
     SV *self;
   };
%}

class PerlWorker : public SwigWorker {
public:
    PerlWorker(const std::string &config);
    virtual ~PerlWorker();
    void register_handler( const std::string & function_name, SV * func = NULL );
    void set_self(SV *s);
};
