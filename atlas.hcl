env "local" {
  url = "postgres://foxy:foxy@/foxy?host=/var/run/postgresql"
  migration {
    dir = "file://migrations"
  }
}

env "dev" {
  url = "postgres://foxy_dev:foxy_dev@/foxy_dev?host=/var/run/postgresql"
  migration {
    dir = "file://migrations"
  }
}

env "prod" {
  url = "postgres://foxy:foxy@/foxy?host=/var/run/postgresql"
  migration {
    dir = "file://migrations"
  }
}
