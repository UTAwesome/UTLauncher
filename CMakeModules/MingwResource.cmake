macro(ADD_WIN32_RESOURCE outfiles _resource)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	if(NOT DEFINED CMAKE_RC_COMPILER)
		find_program(CMAKE_RC_COMPILER i686-pc-mingw32-windres)
		if(CMAKE_RC_COMPILER)
			message(STATUS "Windres found ${CMAKE_RC_COMPILER}")
		else(CMAKE_RC_COMPILER)
			message(SEND_ERROR "Can't compile resources, Windres not found.")
		endif(CMAKE_RC_COMPILER)
	endif(NOT DEFINED CMAKE_RC_COMPILER)

	if(CMAKE_RC_COMPILER)
		get_filename_component(infile ${_resource} ABSOLUTE)
		get_filename_component(outfile ${_resource} NAME)
		set(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.o)

		message(STATUS "Adding dependency ${outfile}")
		add_custom_command(OUTPUT ${outfile}
			COMMAND ${CMAKE_RC_COMPILER} ${infile} -O coff -o ${outfile}
			MAIN_DEPENDENCY ${infile}
		)
#		add_dependencies(${_target} ${CMAKE_CURRENT_BINARY_DIR}/resource.o)
		set(${outfiles} ${${outfiles}} ${outfile})

	else(CMAKE_RC_COMPILER)

	endif(CMAKE_RC_COMPILER)
else(CMAKE_SYSTEM_NAME MATCHES Windows)
	message(STATUS "Ignoring ${_resource} due to not supported platform")
endif(CMAKE_SYSTEM_NAME MATCHES Windows)

endmacro(ADD_WIN32_RESOURCE _resources)