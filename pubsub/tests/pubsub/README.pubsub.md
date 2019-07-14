# Generic PubSub Implementation

## Authentication
Initially I had intended to rely on user authentication via the http spec,
now I am considering doing the auth in the program itself.

Doing http-auth:
+ The web server does all the work of authentication, denial, approval,
  etc. All the cgi program has to do is simply use the name provided by
  the web server.
+ As web auth methods get more secure the cgi program will not have to be
  changed to accomodate them (For example, Mutual Auth, HOBA, AWS-HMAC)
- The cgi program will be tied to a specific web-server as it will (for
  apache) manage the UserAuthFile itself which the webserver will read.
- The security provided by the http specification for authentication may
  be inadequate (md5 sums) and is a risk if the authfile ever gets leaked.

Doing it in the program:
+ Makes the cgi program portable to other web-servers.
- means a whole lot more work just to auth users (check a cookie, lookup a
  session, determine the user).
- If an upgrade occurs to a more secure authentication scheme then the
  cgi program needs to be updated (and will for a time allow old and new
  authentication schemes). This is more work.


## Access control
Initially performed by the admin/installer who must set up an admin
account with which to manage usernames, groups and permissions.
Administrator must manually add the root superuser to the password file
when configuring the service.

Thereafter the users who are in an admin group will be able to create new
users.

## API in brief

### Overview
All responses package the response data into JSON trees and set the
Content-type header to `application/json`.

All JSON fields specified herein are case-sensitive, for both request and
response fields.

Special restrictions apply to the emails and group names that are stored:
1. All emails must contain at least a single '@' character.
2. Group names beginning with underscore are not allowed.

### Session maintenance
All the endpoints below will check the cookie for a session ID and generate
an error if the session ID is missing, or present but invalid. The only
exception is the Login (duh) which needs no session ID.

The caller must perform the requisite authentication via the `Login`
endpoint if the returned error indicates an invalid session.

The cookie name is `session-id` (note case).

### Error handling
All the responses except `queue-get` will include in the root of the reply
two fields (note case):
```javascript
."error-code"      // An integer value containing a the error number
."error-message"   // An english description of the error
```

The `queue-get` indicates errors using http status codes only, as it does
not return a JSON tree.

(TODO: Create a list of error response codes).

### Session management
#### Login
```javascript
POST /login
{
   "email":       "example@email.com",
   "password":    "cleartext password"
}
```
RETURNS:
```javascript
{
   "session-id":  "session ID"      // Also in cookie
}
```

#### Logout
```javascript
POST /logout
{
   // Empty, session is destroyed based on cookie value
}
```
RETURNS:
```javascript
{
   "session-id":     "0000000000000000"  // Also in cookie
}
```


### User, group and permissions management

```javascript
POST /grant
{
   "email":       "example@email.com",
   "perms":       "create-user" // "create-group", "del-user", "del-group"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript

POST /revoke
{
   "email":       "example@email.com",
   "perms":       "create-user" // "create-group",
                                // "delete-user", "delete-group"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /grant-to-user-over-user
{
   "email":       "example@email.com",
   "target-user": "two@example.com",
   "perms":       "modify" // "delete",
                           // "change-permissions",
                           // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /grant-to-user-over-group
{
   "email":          "example@email.com",
   "target-group":   "Group-1",
   "perms":          "modify" // "delete",
                              // "change-permissions",
                              // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /grant-to-group-over-user
{
   "group-name":  "two@example.com",
   "target-user": "example@email.com",
   "perms":       "modify" // "delete",
                           // "change-permissions",
                           // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /grant-to-group-over-group
{
   "group-name":     "two@example.com",
   "target-group":   "example@email.com",
   "perms":          "modify" // "delete",
                              // "change-permissions",
                              // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /revoke-from-user-over-user
{
   "email":       "example@email.com",
   "target-user": "two@example.com",
   "perms":       "modify" // "delete",
                           // "change-permissions",
                           // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /revoke-from-user-over-group
{
   "email":          "example@email.com",
   "target-group":   "Group-1",
   "perms":          "modify" // "delete",
                              // "change-permissions",
                              // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /revoke-from-group-over-user
{
   "group-name":  "two@example.com",
   "target-user": "example@email.com",
   "perms":       "modify" // "delete",
                           // "change-permissions",
                           // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.


```javascript
POST /revoke-from-group-over-group
{
   "group-name":     "two@example.com",
   "target-group":   "example@email.com",
   "perms":          "modify" // "delete",
                              // "change-permissions",
                              // "change-membership"
}
```
RETURNS: "error-code" and "error-message" fields only.




The permissions that can be granted/revoked are:
```javascript
   create            // Create a new instance of <resource>
   read              // Read a specific instance
   update            // Update a specific instance
   delete            // Delete a specific instance
```
The &lt;resources&gt; that can be created are:
```javascript
   user
   group
   permission
   membership
```
For specific instances (read, update and delete), the instance being
managed must be named in the granting/revocation call. The call itself is
specific to the subject, grant/revocation, the permission and the target.
```javascript
   grant[-specific]-to-[subject][-over-[target]]
   revoke[-specific]-from-[subject]-over-[target]

   specific   Optional (see above), either perms or membership
   subject    The entity to grant/revoke the permission (user or group)
   target     The entity which the subject has control over (user or group)

```

This results in the following enumeration of API calls to modify
permissions:
```javascript
   grant-to-user-over-user                  <email> <target> <perms>
   grant-to-user-over-group                 <group> <target> <perms>
   grant-to-group-over-user                 <email> <target> <perms>
   grant-to-group-over-group                <group> <target> <perms>

   grant-perms-to-user-over-user            <email> <target> <perms>
   grant-perms-to-user-over-group           <group> <target> <perms>
   grant-perms-to-group-over-user           <email> <target> <perms>
   grant-perms-to-group-over-group          <group> <target> <perms>

   grant-membership-to-user-over-user       <email> <target> <perms>
   grant-membership-to-user-over-group      <group> <target> <perms>
   grant-membership-to-group-over-user      <email> <target> <perms>
   grant-membership-to-group-over-group     <group> <target> <perms>

   revoke-to-user-over-user                <email> <target> <perms>
   revoke-to-user-over-group               <group> <target> <perms>
   revoke-to-group-over-user               <email> <target> <perms>
   revoke-to-group-over-group              <group> <target> <perms>

   revoke-perms-to-user-over-user          <email> <target> <perms>
   revoke-perms-to-user-over-group         <group> <target> <perms>
   revoke-perms-to-group-over-user         <email> <target> <perms>
   revoke-perms-to-group-over-group        <group> <target> <perms>

   revoke-membership-to-user-over-user     <email> <target> <perms>
   revoke-membership-to-user-over-group    <group> <target> <perms>
   revoke-membership-to-group-over-user    <email> <target> <perms>
   revoke-membership-to-group-over-group   <group> <target> <perms>

```
For example:
```javascript
   // Grant read,update perms to one@example.com for user two@example.com
   grant-to-user-over-user one@example.com read,update two@example.com

   // Grant read,update perms to user one@example.com for users in group
   // Group-1.
   grant-to-user-over-group one@example.com read,update Group-1


   // Grant read,update perms to group Group-1 for user one@example.com
   grant-to-group-over-user Group-1 read,update one@example.com

   // Grant read,update perms to group Group-1 for users in group
   // Group-2.
   grant-to-group-over-group Group-1 read,update Group-2


   // Grant all perms to user one@example.com for permission modifications
   // of user two@example.com
   grant-perms-to-user-over-user one@example.com all two@example.com

   // Grant all perms to user one@example.com for permission modifications
   // of group Group-1
   grant-perms-to-user-over-group one@example.com all Group-1

   // Grant all perms to group Group-1 for permission modifications
   // of group Group-1
   grant-perms-to-group-over-group Group-1 all Group-2


   // Grant update perms to user one@example.com for membership modifications
   // of user two@example.com
   grant-membership-to-user-over-user one@example.com update two@example.com

   // Grant update perms to user one@example.com for membership modifications
   // of group Group-1
   grant-membership-to-user-over-group one@example.com update Group-1

   // Grant update perms to group Group-1 for membership modifications
   // of group Group-1
   grant-membership-to-group-over-group Group-1 update Group-2


```
#### New user
```javascript
POST /user-new
{
   "email":       "example@email.com",
   "nick":        "Nickname to use",
   "password":    "cleartext password"
}
```
RETURNS:
```javascript
{
   "user-id":     "ID of created user"
   "email":       "email of user that was created",
   "nick":        "Nickname of user that was created",
}
```

#### Remove user
```javascript
POST /user-rm
{
   "email":       "example@email.com"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### List users
```javascript
POST /user-list
{
   "email-pattern":     "Pattern to find for for emails",
   "nick-pattern":      "Pattern to find for for nicks",
   "resultset-emails":  "true", // set to false to exclude emails
   "resultset-nicks":   "true", // set to false to exclude nicks
   "resultset-flags":   "true", // set to false to exclude flags
   "resultset-ids":     "true", // set to false to exclude ids
}
```
RETURNS:
```javascript
{
   "email-pattern":     "Pattern matched against email address",
   "nick-pattern":      "Pattern matched against nicknames",
   "id-pattern":        "Pattern matched against IDs",
   "resultset-count":   64,               // Number of users in the results
   "resultset-emails":  [email1, ...]
   "resultset-nicks":   [nick1, ...]
   "resultset-flags":   [flag1, ...]
   "resultset-ids":     [id1, ...]
}
```


#### Modify user
```javascript
POST /user-mod
{
   "old-email":   "old@email.com",
   "new-email":   "new@email.com",
   "nick":        "Nickname to use",      // optional
   "password":    "cleartext password"    // optional
}
```
RETURNS: "error-code" and "error-message" fields only.


#### New group
```javascript
POST /group-new
{
   "group-name":        "group name",
   "group-description": "A description of the group",
}
```
RETURNS:
```javascript
{
   "group-id":     "ID of created group"
   "group-name":   "Name of created group",
   "group-description": "Description created group",
}
```


#### Remove group
```javascript
POST /group-rm
{
   "group-name":        "group name"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Modify group
```javascript
POST /group-mod
{
   "old-group-name":    "Current group name",
   "new-group-name":    "New group name"
   "group-description": "New group description"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Add to group
```javascript
POST /group-adduser
{
   "group-name":  "group name",
   "email":       "email of user to add"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Remove from group
```javascript
POST /group-rmuser
{
   "group-name":  "group name",
   "email":       "email of user to remove"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### List groups
```javascript
POST /group-list
{
   "name-pattern":            "Pattern to find, matches name",
   "description-pattern":     "Pattern to find, matches description",
   "resultset-names":         "true",  // Set to false to exclude
   "resultset-descriptions":  "true"   // Set to false to exclude
   "resultset-ids":           "true"   // Set to false to exclude
}
```
RETURNS:
```javascript
{
   "group-pattern":           "Pattern used for matching",
   "resultset-count":         64,       // Number of groups in the results
   "resultset-names":         [group1, ...],
   "resultset-descriptions":  [description1, ...]
   "resultset-ids":           [id1, ...]
}
```

#### List group members
```javascript
POST /group-members
{
   "group-name":        "Name of group"
   "resultset-emails":  "true", // set to false to exclude emails
   "resultset-nicks":   "true", // set to false to exclude nicks
   "resultset-flags":   "true", // set to false to exclude flags
   "resultset-ids":     "true", // set to false to exclude ids
}
```
RETURNS:
```javascript
{
   "group-name":        "Name of group",
   "resultset-count":   64,               // Number of users in the results
   "resultset-emails":  [email1, ...]
   "resultset-nicks":   [nick1, ...]
   "resultset-flags":   [flag1, ...]
   "resultset-ids":     [id1, ...]
}
```


#### Grant perms to a user
```javascript
POST /perms-grant-user
{
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke perms to a user
```javascript
POST /perms-revoke-user
{
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### List user's perms on resource
```javascript
POST /perms-resource-user
{
   "email":     "email@example.com",
   "resource":  "Resource to be examined"
}
```
RETURNS:
```javascript
{
   "email":             "email@example.com",
   "resultset-count":   64,               // Number of results
   "resultset":         [email1-perms, ...]
}
```


#### Grant perms to a group
```javascript
POST /perms-grant-group
{
   "group-name":  "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke perms to a group
```javascript
POST /perms-revoke-group
{
   "group-name":  "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### List groups's perms on resource
```javascript
POST /perms-resource-group
{
   "group-name":  "Name of group",
   "resource":    "Resource to be examined"
}
```
RETURNS:
```javascript
{
   "group-name":        "Name of group being examined",
   "resultset-count":   64,               // Number of results
   "resultset":         [group1-perms, ...]
}
```

### Queue creation, enqueuing and deleting
#### Add queue
```javascript
POST /queue-new
{
   "queue-name":        "name",
   "queue-description": "A description of the queue",
}
```
RETURNS:
```javascript
{
   "queue-id":     "ID of created queue"
}
```

#### Remove queue
```javascript
POST /queue-rm
{
   "queue-id":        "queue ID",
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Modify queue
```javascript
POST /queue-mod
{
   "queue-id":    "ID of queue to modify",
   ...   // Still haven't decided what properties go into a queue
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Putting a message into a queue
`PUT /queue-put/queue-id`
`<content body>`

RETURNS:
```javascript
{
   "message-id:   "ID of message added to queue"
}
```

#### Getting a message from a queue
`GET /queue-get/queue-id/message-id`

RETURNS: HTTP status code and raw data only
```javascript
<binary data/octet-stream>
```


#### Removing a message from a queue
`DELETE /queue-del/queue-id`
RETURNS: "error-code" and "error-message" fields only.


#### Listing messages in a queue
Retrieves all message ids starting at the specified id
`GET /queue-list/queue-id/message-id`

RETURNS:
```javascript
{
   "message-ids":    [message-id, ...]
}
```


## Implementation in brief

### Data storage
Once the subscriber list grows to any appreciable size doing just the
permissions lookups will take time unless a dedicated indexed storage is used.

Unfortunately this means at least one more dependency: a RDBMS. To this
end the dependency is [libsqldb](https://github.com/lelanthran/libsqldb),
with a minimum version of v0.1.4.

### Queues
Queues will be implemented using a combination of the database and the
filesystem - the metadata and a filename is written to the database, the
queue contents is written to the file. The use of files is to enable very
large messages to be processed/stored.

"Listening" on a queue must result in a SSE GET request (not yet in the
spec above), while the usual requests will be used for examining the queue,
removing items from a queue, etc.

Metadata for a queue must include, at a minimum:
1. Queue name.
2. Queue description.
3. An expiry based on inactivity for the queue.
4. An expiry for messages (expired items expunged).
5. A maximum queue length (older items expunged).
6. An optional password for posting to a queue.
7. An optional password for reading from a queue.
8. List of listeners.

Arbitrary metadata (set by callers) in the form name/value pairs must be
supported.

There must be a few default queues (for example, a queue that will email the
contents. Useful for notifying users of actions).


### Sample usage
1. Client #1: create queue "ChessName1", metadata parameters (rndcolor=b)
2. Client #1: post "notify-user: email, ChessName1" to /sendmail/
3. Client #1: listen sse to /ChessName1
4. Client #2: post "joined" to /ChessName1
5. Client #2: listen sse to /ChessName1
6. Client #1: reads "ready"
7. Client #1: listen sse for move
8. Client #2: post "1. move#" to /ChessName1
9. Client #1: reads "1. move#"
9. Client #2: post "2. move#" to /ChessName1
