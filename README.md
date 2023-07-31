# webserv

Test hostname:
curl -H "Host: www.default.com" http://localhost:8080/

Test pending request:
curl -X POST -H "Content-Length: 40" --data "first_name=ad&last_name=carnec" localhost:
8080

Ressources:

* Socket programming, Nonblocking I/O and select() : https://www.ibm.com/docs/en/i/7.1?topic=designs-example-nonblocking-io-select

* Siege (stress test) : https://www.systutorials.com/docs/linux/man/1-siege/

* PUT : https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/PUT

* POST : https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/POST

