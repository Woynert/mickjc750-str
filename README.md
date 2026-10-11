Original: https://github.com/mickjc750/str

### Differences with original

* In the original, capacity doesn't account for the null terminator. Here, it does.
* Here strbuf_t is supposed to be owned as an instance and passed around as a apointer.
 Unlike in the original where you are supposed to pass it around as a pointer to pointer.

### Tests

```
cd tests
make setupSetupDebug
make compile && make test
```

