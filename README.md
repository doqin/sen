# Sen programming language
A vietnamese syntax compiled programming language in the making

```go
hàm kiểm_tra(a: số, b: số) {
    nếu (a == b) {
        in("đúng");
    } không thì {
        in("sai");
    }
}

hàm chính() {
    biến a: số;
    a = 2;
    biến b = 3;
    kiểm_tra(a, b);
}
```

## Current state
Work in progress
- [x] Lexer
- [x] Parser
- [ ] Semantic Analysis
