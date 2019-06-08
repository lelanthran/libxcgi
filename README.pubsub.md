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

#### Remove user
```javascript
POST /user-rm
{
   "email":       "example@email.com",
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

#### New group
```javascript
POST /group-new
{
   "name":        "group name",
   "description": "A description of the group",
}
```

#### Remove group
```javascript
POST /group-rm
{
   "name":        "group name",
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

#### Revoke perms to a user
```javascript
POST /perms-revoke-user
{
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```

#### Grant perms to a group
```javascript
POST /perms-grant-group
{
   "name":        "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```

#### Revoke perms to a group
```javascript
POST /perms-revoke-group
{
   "name":        "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```

### Queue creation, enqueuing and deleting
#### Queue creation
```javascript
POST /queue-create
{
   "name":        "name",
   "description": "A description of the queue",
}
```

#### Listing messages in a queue
Retrieves all message ids starting at the specified id
`GET /queue-list/queue-id/message-id`

RETURNS
```javascript
   {
      "message-ids":    [message-id, ...]
   }
```

#### Putting a message into a queue
`PUT /queue-put/queue-id`
`<content body>`

RETURNS
```javascript
{
   "message-id:   "ID of message added to queue"
}
```

#### Getting a message from a queue
`GET /queue-get/queue-id/message-id`

RETURNS
`<binary data/octet-stream>`


#### Removing a message from a queue
`DELETE /queue-del/queue-id`



## Implementation in brief

### Data storage
Once the subscriber list grows to any appreciable size doing just the
permissions lookups will take time unless a dedicated indexed storage is used.

Unfortunately this means at least one more dependency: sqlite.

[**NOTE**: All the unique clumns should be indexed]

```SQL
   CREATE TABLE tuser (
      cid:     INTEGER PRIMARY KEY,
      cemail:  TEXT UNIQUE,
      cnick:   TEXT);

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

