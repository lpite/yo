FROM clojure:temurin-21-tools-deps-alpine

WORKDIR /app

COPY deps.edn ./
COPY src ./src

EXPOSE 3000

CMD ["clojure", "-M:run"]