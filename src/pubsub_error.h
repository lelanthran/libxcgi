
#ifndef H_PUBSUB_ERROR
#define H_PUBSUB_ERROR

#define EPUBSUB_SUCCESS    (0)
#define EPUBSUB_AUTH       (1)

#ifdef __cplusplus
extern "C" {
#endif

   const char *pubsub_error_msg (int errorcode);


#ifdef __cplusplus
};
#endif


#endif
