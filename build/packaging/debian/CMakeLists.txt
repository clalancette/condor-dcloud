#Modify timestamp in changelog file
PackageDate( DEB DEB_DATE)

configure_file( changelog.in changelog.Debian @ONLY )

add_custom_target( generate_changelog
		   ALL
		   WORKING_DIRECTORY	${CMAKE_CURRENT_SOURCE_DIR}
		   COMMAND rm -f changelog.Debian.gz
		   COMMAND gzip -9c changelog.Debian > changelog.Debian.gz )

install(FILES		copyright
        DESTINATION	"${C_DOC}" )

install(FILES		changelog.Debian.gz
        DESTINATION	"${C_DOC}" )

