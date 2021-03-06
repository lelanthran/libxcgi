# Generic PubSub Implementation

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
All the responses except `binary-get` will include in the root of the reply
two fields (note case):
```javascript
."error-code"      // An integer value containing a the error number
."error-message"   // An english description of the error
```

The `binary-get` indicates errors using http status codes only, as it does
not return a JSON tree.

(TODO: `binary-get` not planned for near future)

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


### User flags
A set of flags are stored for each user. These flags can be set and read
when the permissions `modify-user` is granted. As of writing, only the
`lockout` flag is recognised. Other flags are simply ignored.

#### Set user flags
```javascript
POST /flags-set
{
   "email":       "example@email.com",
   "flags":       "lockout,..." // More TBA
}
```
RETURNS: "error-code" and "error-message" fields only.

#### Clear user flags
```javascript
POST /flags-clear
{
   "email":       "example@email.com",
   "flags":       "lockout,..." // More TBA
}
```
RETURNS: "error-code" and "error-message" fields only.


### User, group and permissions management
Very fine-grained user, group and ACL management is possible. In addition
to managing the user and group ACLS, caller-defined resources can be also
managed (see below, `Resource management`).

1. Users are members of one or more groups.
2. User inherit all of the permissions of the groups they are in.
3. Users can be granted permissions to create users and/or groups.
4. Groups can be granted permissions to create users and/or groups.
5. Users can be granted specific permissions to read, modify, grant/revoke
   permissions or delete a specific instance of another user.
6. Users can be granted specific permissions to read, modify, grant/revoke
   permissions, delete or list any members of a specific group.
7. Groups can be granted specific permissions to read, modify, grant/revoke
   permissions or delete a specific instance of a user.
8. Groups can be granted specific permissions to read, modify, grant/revoke
   permissions, delete or list any members of a specific group.

In practice it is simpler to manage ACL only via groups (role-based ACL).

#### Create a resource (object which can be managed using permissions)
```
POST /resource-new
{
   "resource":    "..." // Caller-defined
}
```
RETURNS: "error-code" and "error-message" fields only.

#### Delete a resource
```
POST /resource-rm
{
   "resource":    "..." // Caller-defined
}
```
RETURNS: "error-code" and "error-message" fields only.



#### Grant permissions to a user for a resource
```javascript
POST /grant-to-user
{
   "email":       "example@email.com",
   "perms":       "0,..."  // Up to 32 bits can be used (0..31)
   "resource":    "..." // Caller-defined
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke permissions from a user for a resource
```javascript
POST /revoke-from-user
{
   "email":       "example@email.com",
   "perms":       "0,..."  // Up to 32 bits can be used (0..31)
   "resource":    "..." // Caller-defined
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Get permissions for a user
```javascript
POST /perms-for-user
{
   "email":       "example@email.com",
   "resource":    "..." // Caller-defined
}
```
RETURNS:
```javascript
{
   "email":          "example@email.com",
   "resource":       "..." // caller-defined
   "perms":          "0,1,..." (0..31)
}
```


#### Grant permissions to a group
```javascript
POST /grant-to-group
{
   "group-name":  "Group-1",
   "perms":       "0,..."  // Up to 32 bits can be used (0..31)
   "resource":    "..." // Caller-defined
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke permissions from a group
```javascript
POST /revoke-from-group
{
   "group-name":  "Group-1",
   "perms":       "0,..."  // Up to 32 bits can be used (0..31)
   "resource":    "..." // Caller-defined
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Get permissions for a group
```javascript
POST /perms-for-group
{
   "group-name":  "Group-1",
   "resource":    "..." // Caller-defined
}
```
RETURNS:
```javascript
{
   "email":          "example@email.com",
   "resource":       "..." // caller-defined
   "perms":          "0,1,..." (0..31)
}
```






#### Grant creation permissions to a user
```javascript
POST /grant-create-to-user
{
   "email":       "example@email.com",
   "perms":       "create-user,..." // "create-group",
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke creation permissions from a user
```javascript
POST /revoke-create-from-user
{
   "email":       "example@email.com",
   "perms":       "create-user,..." // "create-group",
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Grant creation permissions to a group
```javascript
POST /grant-create-to-group
{
   "group-name":  "Group-1",
   "perms":       "create-user,..." // "create-group",
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke creation permissions from a group
```javascript
POST /revoke-create-from-group
{
   "group-name":  "Group-1",
   "perms":       "create-user,..." // "create-group",
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Get creation permissions for a user
```javascript
POST /perms-user
{
   "email":       "example@email.com",
}
```
RETURNS:
```javascript
{
   "email":       "example@email.com",
   "perms":       "create-group,create-user,..."
}
```

#### Get creation permissions for a group
```javascript
POST /perms-group
{
   "group-name":  "Group-1"
}
```
RETURNS:
```javascript
{
   "group-name":  "Group-1"
   "perms":       "create-group,create-user,..."
}
```

#### Get permissions for a user over another user
```javascript
POST /perms-user-over-user
{
   "email":             "example@email.com",
   "target-user":       "two@email.com"
}
```
RETURNS:
```javascript
{
   "email":             "example@email.com",
   "target-user":       "two@email.com",
   "perms":             "modify-user,delete-user,..."
}
```

#### Get permissions for a user over a group
```javascript
POST /perms-user-over-group
{
   "email":             "example@email.com",
   "target-group":      "Group-Name"
}
```
RETURNS:
```javascript
{
   "email":             "example@email.com",
   "target-group":      "Group-Name",
   "perms":             "modify-user,delete-user,..."
}
```

#### Get permissions for a group over a user
```javascript
POST /perms-user-over-group
{
   "group-name":        "example@email.com",
   "target-user":       "two@email.com"
}
```
RETURNS:
```javascript
{
   "email":             "example@email.com",
   "target-user":       "two@email.com",
   "perms":             "modify-user,delete-user,..."
}
```

#### Get permissions for a group over a group
```javascript
POST /perms-group-over-group
{
   "group-name":        "example@email.com",
   "target-group":      "Group-1"
}
```
RETURNS:
```javascript
{
   "email":             "example@email.com",
   "target-group":      "Group-1",
   "perms":             "modify-user,delete-user,..."
}
```

#### Grant permissions to a user over another user
```javascript
POST /grant-to-user-over-user
{
   "email":       "example@email.com",
   "target-user": "two@example.com",
   "perms":       "modify-user" // "delete-user",
                           // "change-permissions",
                           // "change-membership",
                           // "read"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Grant permissions to a user over a group
```javascript
POST /grant-to-user-over-group
{
   "email":          "example@email.com",
   "target-group":   "Group-1",
   "perms":          "modify-user" // "delete-user",
                              // "change-permissions",
                              // "change-membership",
                              // "read",
                              // "list-members"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Grant permissions to a group over a user
```javascript
POST /grant-to-group-over-user
{
   "group-name":  "two@example.com",
   "target-user": "example@email.com",
   "perms":       "modify-user" // "delete-user",
                           // "change-permissions",
                           // "change-membership",
                           // "read"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Grant permissions to a group over another group
```javascript
POST /grant-to-group-over-group
{
   "group-name":     "two@example.com",
   "target-group":   "example@email.com",
   "perms":          "modify-user" // "delete-user",
                              // "change-permissions",
                              // "change-membership",
                              // "read",
                              // "list-members"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke permissions from a user over another user
```javascript
POST /revoke-from-user-over-user
{
   "email":       "example@email.com",
   "target-user": "two@example.com",
   "perms":       "modify-user" // "delete-user",
                           // "change-permissions",
                           // "change-membership",
                           // "read"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke permissions from a user over a group
```javascript
POST /revoke-from-user-over-group
{
   "email":          "example@email.com",
   "target-group":   "Group-1",
   "perms":          "modify-user" // "delete-user",
                              // "change-permissions",
                              // "change-membership",
                              // "read",
                              // "list-members"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke permissions from a group over a user
```javascript
POST /revoke-from-group-over-user
{
   "group-name":  "two@example.com",
   "target-user": "example@email.com",
   "perms":       "modify-user" // "delete-user",
                           // "change-permissions",
                           // "change-membership",
                           // "read"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke permissions from a group over another group
```javascript
POST /revoke-from-group-over-group
{
   "group-name":     "two@example.com",
   "target-group":   "example@email.com",
   "perms":          "modify-user" // "delete-user",
                              // "change-permissions",
                              // "change-membership",
                              // "read",
                              // "list-members"
}
```
RETURNS: "error-code" and "error-message" fields only.




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


#### User info
```javascript
POST /user-info
{
   "email":       "example@email.com"
}
```
RETURNS:
```javascript
{
   "email":     "one@example.com",
   "nick":      "User One",
   "flags":     "lockout", // ...
   "user-id":   "12"
}
```


#### List users
```javascript
POST /user-find
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
   "resultset-emails":  [email1, ...],
   "resultset-nicks":   [nick1, ...],
   "resultset-flags":   [flag1, ...],
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
POST /group-find
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
   "resultset-descriptions":  [description1, ...],
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
   "resultset-emails":  [email1, ...],
   "resultset-nicks":   [nick1, ...],
   "resultset-flags":   [flag1, ...],
   "resultset-ids":     [id1, ...]
}
```


#### Grant perms to a user
```javascript
POST /perms-grant-user
{
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A resource, user or group"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke perms to a user
```javascript
POST /perms-revoke-user
{
   "email":       "example@email.com",
   "perms":       "See list of perms allowed",
   "resource":    "A resource, user or group"
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
   "resource":    "A resource, user or group"
}
```
RETURNS: "error-code" and "error-message" fields only.


#### Revoke perms to a group
```javascript
POST /perms-revoke-group
{
   "group-name":  "group name",
   "perms":       "See list of perms allowed",
   "resource":    "A resource, user or group"
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


### Data definitions








## Implementation in brief

### Data storage
Once the subscriber list grows to any appreciable size doing just the
permissions lookups will take time unless a dedicated indexed storage is used.

Unfortunately this means at least one more dependency: a RDBMS. To this
end the dependency is [libsqldb](https://github.com/lelanthran/libsqldb),
with a minimum version of v0.1.5.

### Data manipulation
Clients must create a database, then create tables within this database,
and finally rows within the tables.

The database, table and column names are very restricted; this is to
prevent SQL injection attacks (it is not possible to parameterise table
creation).

new_database : {
   "database-name" : "database_name"
}

new_table : {
   "database-name":  "database_name",
   "name":           "Table_Name",
   "description":    "Table Description",
   "columns":
      [
         {
            "column-name":    "Column_Name_1",
            "attributes":     "unique,primary,indexed",
            "type":           "text | integer | key",
         },
         {
            "column-name":    "Column_Name_2",
            "attributes":     "unique,primary,indexed",
            "type":           "text | integer | key",
         },
         ...
      ],
   "keys":
      [
         {
            "local-column":   "Column_Name",
            "foreign-table":  "Table_Name",
            "foreign-column": "Column_Name",
         },
         {
            "local-column":   "Column_Name",
            "foreign-table":  "Table_Name",
            "foreign-column": "Column_Name",
         },
      ],
}

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
