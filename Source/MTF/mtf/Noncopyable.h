#ifndef noncopyable_h
#define noncopyable_h

#define MTF_MATK_NONCOPYABLE(className)  \
	private: \
	    className(const ClassName&) ;\
	    className& operator=(const ClassName&) ;



#endif // nocopyable_h
