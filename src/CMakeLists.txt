target_sources(
        las_c
        PRIVATE
        dest.c
        header.c
        las.c
        point.c
        reader.c
        source.c
        writer.c
)

add_subdirectory(private)