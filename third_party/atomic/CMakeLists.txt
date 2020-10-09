set(SRC_HEADERS
	intern/atomic_ops_ext.h
	intern/atomic_ops_msvc.h
	intern/atomic_ops_unix.h
	intern/atomic_ops_utils.h
)

install(FILES "atomic_ops.h" DESTINATION "include")
install(FILES ${SRC_HEADERS} DESTINATION "include/intern")
