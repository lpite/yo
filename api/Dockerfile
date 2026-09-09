FROM clojure:tools-deps-1.11.1-alpine

WORKDIR /app

COPY deps.edn ./
COPY src ./src

EXPOSE 3000

CMD ["clojure", "-M:run"]