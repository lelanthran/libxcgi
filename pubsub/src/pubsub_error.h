
#ifndef H_PUBSUB_ERROR
#define H_PUBSUB_ERROR

#define EPUBSUB_SUCCESS          (0)

// The first eight bytes are reserved for specific errors
#define EPUBSUB_UNIMPLEMENTED    (1 << 8)
#define EPUBSUB_INTERNAL_ERROR   (2 << 8)
#define EPUBSUB_ENDPOINT         (3 << 8)
#define EPUBSUB_MISSING_PARAMS   (4 << 8)
#define EPUBSUB_BAD_PARAMS       (5 << 8)
#define EPUBSUB_NOT_AUTH         (6 << 8)
#define EPUBSUB_AUTH_FAILURE     (7 << 8)

#ifdef __cplusplus
extern "C" {
#endif

   const char *pubsub_error_msg (int errorcode);


#ifdef __cplusplus
};
#endif


#endif

