# Generic PubSub Implementation

## Authentication
Authentication must be performed by the web server which, even if md5sum
hashes are used, will be suitable over https.

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
POST
{
   "action":      "add-user",
   "email":       "example@email.com",
   "nick":        "Nickname to use",
   "password":    "cleartext password"
}
```

#### Remove user
```javascript
POST
{
   "action":      "rm-user",
   "email":       "example@email.com",
}
```

#### Modify user
```javascript
POST
{
   "action":      "mod-user",
   "old-email":   "old@email.com",
   "new-email":   "new@email.com",
   "nick":        "Nickname to use",      // optional
   "password":    "cleartext password"    // optional
}
```

#### New group
```javascript
POST
{
   "action":      "add-group",
   "name":        "group name",
}
```

#### Remove group
```javascript
POST
{
   "action":      "rm-group",
   "name":        "group name",
}
```

#### Grant perms to a user
```javascript
POST
{
   "action":      "grant-user",
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```

#### Revoke perms to a user
```javascript
POST
{
   "action":      "revoke-user",
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```

#### Grant perms to a group
```javascript
POST
{
   "action":      "grant-user",
   "name":        "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```

#### Revoke perms to a group
```javascript
POST
{
   "action":      "revoke-user",
   "name":        "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A queue, user or group"
}
```

### Queue creation, enqueuing and deleting
#### Queue creation
```javascript
POST
{
   "action":      "queue-create",
   "name":        "name",
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
      cid:     INTEGER PRIMARY KEY,
      cname:   TEXT UNIQUE);

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

