

/* Allow this file to be included multiple times
   with different settings of NDEBUG.  */
#undef assert

#ifdef NDEBUG
#define assert(ignore)
#else


#define assert(ex)						\
	( (ex) ? 1 :						\
		shutdown( "Failed assertion " #ex " at line %d of `%s'.\n", \
		__LINE__, __FILE__ ) )

#endif  // NDEBUG
