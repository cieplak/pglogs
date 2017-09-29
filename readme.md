pglogs
------

Usage:

```
pg_recvlogical --slot my_replication_slot  --start -f - | pglogs
```

compiling
---------

```
make deps
make
cp bin/pglogs <directory in your $PATH>
```
