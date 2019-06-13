
#ifndef H_PUBSUB_ERROR
#define H_PUBSUB_ERROR

#define EPUBSUB_SUCCESS          (0)

// The first eight bytes are reserved for specific errors
#define EPUBSUB_ENDPOINT         (1 << 8)
#define EPUBSUB_AUTH             (2 << 8)
#define EPUBSUB_BAD_PARAMS       (3 << 8)
#define EPUBSUB_UNIMPLEMENTED    (4 << 8)

#ifdef __cplusplus
extern "C" {
#endif

   const char *pubsub_error_msg (int errorcode);


#ifdef __cplusplus
};
#endif


#endif

