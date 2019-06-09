# Generic PubSub Implementation

## Authentication
Initially I had intended to rely on user authentication via the http spec,
now I am reconsidering doing the auth in the program itself.

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

For now, using the web server's authentication.

*(TODO: The cgi program must manually do the addition/removal of hashes in
the password file being used by apache)*


## Access control
Initially performed by the admin/installer who must set up an admin
account with which to manage usernames, groups and permissions.
Administrator must manually add the root superuser to the password file
when configuring the service.

Thereafter the users who are in an admin group will be able to create new
users.

## API in brief

### Session maintenance
All the endpoints below will check the cookie for session ID and generate
an error if the session ID is missing or present but invalid. The only
exception is the Login (duh) which needs no session ID.

The caller must perform the requisite authentication via the `Login`
endpoint if the returned error indicates an invalid session

### Error handling
All the responses except `queue-get` will include in the root of the reply
two fields (note case):
```javascript
."errorCode"      // An integer value containing a the error number
."errorMessage"   // An english description of the error
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
   "session":     "session ID"      // Also in cookie
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
   "session":     "0000000000000000"  // Also in cookie
}
```


### User, group and permissions management
The list of permissions that can be granted/revoked is:
```javascript
   create-queue      // Create a new message queue
   remove-queue      // Remove an existing message queue
   put               // Put a new message into a message queue
   get               // Retrieve a message from a message queue
   list              // List messages in a message queue
   del               // Remove a message from the message queue
   grant             // Grant a permission to a resource
   revoke            // Revoke a permission from a resource
   create-user       // Create a new user
   create-group      // Create a new group
   mod-user          // Modify a user
   mod-group         // Modify a group
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
}
```

#### Remove user
```javascript
POST /user-rm
{
   "email":       "example@email.com",
}
```
RETURNS: HTTP status code only


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
RETURNS: HTTP status code only


#### New group
```javascript
POST /group-new
{
   "name":        "group name",
   "description": "A description of the group",
}
```
RETURNS:
```javascript
{
   "group-id":     "ID of created group"
}
```

#### Remove group
```javascript
POST /group-rm
{
   "name":        "group name",
}
```
RETURNS: HTTP status code only


#### Grant perms to a user
```javascript
POST /perms-grant-user
{
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: HTTP status code only


#### Revoke perms to a user
```javascript
POST /perms-revoke-user
{
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: HTTP status code only


#### Grant perms to a group
```javascript
POST /perms-grant-group
{
   "name":        "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: HTTP status code only


#### Revoke perms to a group
```javascript
POST /perms-revoke-group
{
   "name":        "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```
RETURNS: HTTP status code only


### Queue creation, enqueuing and deleting
#### Queue creation
```javascript
POST /queue-create
{
   "name":        "name",
   "description": "A description of the queue",
}
```
RETURNS:
```javascript
{
   "queue-id":     "ID of created queue"
}
```

#### Listing messages in a queue
Retrieves all message ids starting at the specified id
`GET /queue-list/queue-id/message-id`

RETURNS:
```javascript
   {
      "message-ids":    [message-id, ...]
   }
```

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

RETURNS:
```javascript
<binary data/octet-stream>
```


#### Removing a message from a queue
`DELETE /queue-del/queue-id`
RETURNS: HTTP status code only



## Implementation in brief

### Data storage
Once the subscriber list grows to any appreciable size doing just the
permissions lookups will take time unless a dedicated indexed storage is used.

Unfortunately this means at least one more dependency: sqlite.

[**NOTE**: All the unique columns should be indexed]

```SQL
   CREATE TABLE tuser (
      cid:     INTEGER PRIMARY KEY,
      cemail:  TEXT UNIQUE,
      cnick:   TEXT,
      session: TEXT,
      salt:    TEXT,
      hash:    TEXT);

   CREATE TABLE tgroup (
      cid:        INTEGER PRIMARY KEY,
      cname:      TEXT UNIQUE,
      description: TEXT);

   CREATE TABLE tgroup_membership (
      cuser:   INTEGER,
      cgroup:  INTEGER,
         FOREIGN KEY (cuser) REFERENCES (tuser.cid),
         FOREIGN KEY (cgroup) REFERENCES (tgroup.cid));

   CREATE TABLE tuser_perm (
      cuser:      INTEGER,
      cperms:     INTEGER,
      cresource:  TEXT,
         FOREIGN KEY (cuser) REFERENCES (tuser.cid));

   CREATE TABLE tgroup_perm (
      cgroup:     INTEGER,
      cperms:     INTEGER,
      cresource:  TEXT,
         FOREIGN KEY (cgroup) REFERENCES (tgroup.cid));

```

