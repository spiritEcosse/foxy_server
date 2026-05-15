env {
  name = atlas.env
  url  = "postgres://foxy@/foxy?host=/var/run/postgresql&sslmode=disable"
  migration {
    dir = "file://migrations"
  }
}
