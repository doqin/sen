# Sen programming language
A vietnamese syntax compiled programming language in the making

```go
hàm kiểm_tra(a: số nguyên, b: số nguyên): luận lý {
    nếu (a == b)
        trả đúng;
    trả sai;
}

hàm chính() {
    biến a: số thực = 8.2;
    biến b: chuỗi = "xin chào";
    biến c: luận lý = đúng;
    biến d: số nguyên = 8;
    biến e: số nguyên = 6;

    biến kq = kiểm_tra(d, e);
    in(kq);
}
```

## Current state
Work in progress
- [x] Lexer
- [x] Parser
- [ ] Semantic Analysis
