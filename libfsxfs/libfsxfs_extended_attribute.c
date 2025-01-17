/*
 * Extended attribute functions
 *
 * Copyright (C) 2020-2023, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include "libfsxfs_attribute_values.h"
#include "libfsxfs_attributes.h"
#include "libfsxfs_extended_attribute.h"
#include "libfsxfs_extent.h"
#include "libfsxfs_file_system.h"
#include "libfsxfs_inode.h"
#include "libfsxfs_io_handle.h"
#include "libfsxfs_libbfio.h"
#include "libfsxfs_libcerror.h"
#include "libfsxfs_libcnotify.h"
#include "libfsxfs_libcthreads.h"
#include "libfsxfs_libfdata.h"
#include "libfsxfs_types.h"

/* Creates an extended_attribute
 * Make sure the value extended_attribute is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_initialize(
     libfsxfs_extended_attribute_t **extended_attribute,
     libfsxfs_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     libfsxfs_file_system_t *file_system,
     libfsxfs_inode_t *inode,
     libfsxfs_attribute_values_t *attribute_values,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_initialize";

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	if( *extended_attribute != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid extended attribute value already set.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = memory_allocate_structure(
	                               libfsxfs_internal_extended_attribute_t );

	if( internal_extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create extended attribute.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_extended_attribute,
	     0,
	     sizeof( libfsxfs_internal_extended_attribute_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear extended attribute.",
		 function );

		memory_free(
		 internal_extended_attribute );

		return( -1 );
	}
	internal_extended_attribute->io_handle        = io_handle;
	internal_extended_attribute->file_io_handle   = file_io_handle;
	internal_extended_attribute->file_system      = file_system;
	internal_extended_attribute->inode            = inode;
	internal_extended_attribute->attribute_values = attribute_values;

#if defined( HAVE_MULTI_THREAD_SUPPORT ) && !defined( HAVE_LOCAL_LIBFSXFS )
	if( libcthreads_read_write_lock_initialize(
	     &( internal_extended_attribute->read_write_lock ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to initialize read/write lock.",
		 function );

		goto on_error;
	}
#endif
	*extended_attribute = (libfsxfs_extended_attribute_t *) internal_extended_attribute;

	return( 1 );

on_error:
	if( internal_extended_attribute != NULL )
	{
		memory_free(
		 internal_extended_attribute );
	}
	return( -1 );
}

/* Frees an extended attribute
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_free(
     libfsxfs_extended_attribute_t **extended_attribute,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_free";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	if( *extended_attribute != NULL )
	{
		internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) *extended_attribute;
		*extended_attribute         = NULL;

#if defined( HAVE_MULTI_THREAD_SUPPORT ) && !defined( HAVE_LOCAL_LIBFSXFS )
		if( libcthreads_read_write_lock_free(
		     &( internal_extended_attribute->read_write_lock ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free read/write lock.",
			 function );

			result = -1;
		}
#endif
		if( internal_extended_attribute->data_stream != NULL )
		{
			if( libfdata_stream_free(
			     &( internal_extended_attribute->data_stream ),
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free data stream.",
				 function );

				result = -1;
			}
		}
		memory_free(
		 internal_extended_attribute );
	}
	return( result );
}

/* Retrieves the size of the UTF-8 encoded name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_utf8_name_size(
     libfsxfs_extended_attribute_t *extended_attribute,
     size_t *utf8_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_utf8_name_size";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_attribute_values_get_utf8_name_size(
	     internal_extended_attribute->attribute_values,
	     utf8_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve size of UTF-8 formatted name.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the UTF-8 encoded name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_utf8_name(
     libfsxfs_extended_attribute_t *extended_attribute,
     uint8_t *utf8_string,
     size_t utf8_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_utf8_name";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_attribute_values_get_utf8_name(
	     internal_extended_attribute->attribute_values,
	     utf8_string,
	     utf8_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-8 formatted name.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the size of the UTF-16 encoded name
 * The returned size includes the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_utf16_name_size(
     libfsxfs_extended_attribute_t *extended_attribute,
     size_t *utf16_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_utf16_name_size";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_attribute_values_get_utf16_name_size(
	     internal_extended_attribute->attribute_values,
	     utf16_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve size of UTF-16 formatted name.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the UTF-16 encoded name
 * The size should include the end of string character
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_utf16_name(
     libfsxfs_extended_attribute_t *extended_attribute,
     uint16_t *utf16_string,
     size_t utf16_string_size,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_utf16_name";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_attribute_values_get_utf16_name(
	     internal_extended_attribute->attribute_values,
	     utf16_string,
	     utf16_string_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve UTF-16 formatted name.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_read(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for reading.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Determines the data stream
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_internal_extended_attribute_get_data_stream(
     libfsxfs_internal_extended_attribute_t *internal_extended_attribute,
     libcerror_error_t **error )
{
	static char *function = "libfsxfs_internal_extended_attribute_get_data_stream";

	if( internal_extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	if( internal_extended_attribute->data_stream != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid extended attribute - data stream value already set.",
		 function );

		return( -1 );
	}
	if( libfsxfs_attributes_get_value_data_stream(
	     internal_extended_attribute->io_handle,
	     internal_extended_attribute->inode,
	     internal_extended_attribute->attribute_values,
	     &( internal_extended_attribute->data_stream ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to retrieve value data stream.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Reads data at the current offset into a buffer
 * Returns the number of bytes read or -1 on error
 */
ssize_t libfsxfs_extended_attribute_read_buffer(
         libfsxfs_extended_attribute_t *extended_attribute,
         void *buffer,
         size_t buffer_size,
         libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_read_buffer";
	ssize_t read_count                                                  = 0;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( internal_extended_attribute->data_stream == NULL )
	{
		if( libfsxfs_internal_extended_attribute_get_data_stream(
		     internal_extended_attribute,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine data stream.",
			 function );

			read_count = -1;
		}
	}
	if( internal_extended_attribute->data_stream != NULL )
	{
		read_count = libfdata_stream_read_buffer(
		              internal_extended_attribute->data_stream,
		              (intptr_t *) internal_extended_attribute->file_io_handle,
		              (uint8_t *) buffer,
		              buffer_size,
		              0,
		              error );

		if( read_count < 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read buffer from data stream.",
			 function );

			read_count = -1;
		}
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( read_count );
}

/* Reads data at a specific offset
 * Returns the number of bytes read or -1 on error
 */
ssize_t libfsxfs_extended_attribute_read_buffer_at_offset(
         libfsxfs_extended_attribute_t *extended_attribute,
         void *buffer,
         size_t buffer_size,
         off64_t offset,
         libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_read_buffer_at_offset";
	ssize_t read_count                                                  = 0;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( internal_extended_attribute->data_stream == NULL )
	{
		if( libfsxfs_internal_extended_attribute_get_data_stream(
		     internal_extended_attribute,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine data stream.",
			 function );

			read_count = -1;
		}
	}
	if( internal_extended_attribute->data_stream != NULL )
	{
		read_count = libfdata_stream_read_buffer_at_offset(
		              internal_extended_attribute->data_stream,
		              (intptr_t *) internal_extended_attribute->file_io_handle,
		              (uint8_t *) buffer,
		              buffer_size,
		              offset,
		              0,
		              error );

		if( read_count < 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read buffer at offset from data stream.",
			 function );

			read_count = -1;
		}
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( read_count );
}

/* Seeks a certain offset
 * Returns the offset if seek is successful or -1 on error
 */
off64_t libfsxfs_extended_attribute_seek_offset(
         libfsxfs_extended_attribute_t *extended_attribute,
         off64_t offset,
         int whence,
         libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_seek_offset";

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( internal_extended_attribute->data_stream == NULL )
	{
		if( libfsxfs_internal_extended_attribute_get_data_stream(
		     internal_extended_attribute,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine data stream.",
			 function );

			offset = -1;
		}
	}
	if( internal_extended_attribute->data_stream != NULL )
	{
		offset = libfdata_stream_seek_offset(
		          internal_extended_attribute->data_stream,
		          offset,
		          whence,
		          error );

		if( offset < 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_SEEK_FAILED,
			 "%s: unable to seek offset in data stream.",
			 function );

			offset = -1;
		}
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( offset );
}

/* Retrieves the current offset
 * Returns the offset if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_offset(
     libfsxfs_extended_attribute_t *extended_attribute,
     off64_t *offset,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_offset";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( internal_extended_attribute->data_stream == NULL )
	{
		if( libfsxfs_internal_extended_attribute_get_data_stream(
		     internal_extended_attribute,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to determine data stream.",
			 function );

			result = -1;
		}
	}
	if( internal_extended_attribute->data_stream != NULL )
	{
		if( libfdata_stream_get_offset(
		     internal_extended_attribute->data_stream,
		     offset,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve offset from data stream.",
			 function );

			result = -1;
		}
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves the size of the data stream object
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_size(
     libfsxfs_extended_attribute_t *extended_attribute,
     size64_t *size,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_size";

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

	if( internal_extended_attribute->attribute_values == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid extended attribute - missing attribute values.",
		 function );

		return( -1 );
	}
	if( size == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid size.",
		 function );

		return( -1 );
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	*size = (size64_t) internal_extended_attribute->attribute_values->value_data_size;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( 1 );
}

/* Retrieves the number of extents
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_number_of_extents(
     libfsxfs_extended_attribute_t *extended_attribute,
     int *number_of_extents,
     libcerror_error_t **error )
{
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_number_of_extents";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_inode_get_number_of_attributes_extents(
	     internal_extended_attribute->inode,
	     number_of_extents,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of attribute extents.",
		 function );

		result = -1;
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

/* Retrieves a specific extent
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_extended_attribute_get_extent_by_index(
     libfsxfs_extended_attribute_t *extended_attribute,
     int extent_index,
     off64_t *extent_offset,
     size64_t *extent_size,
     uint32_t *extent_flags,
     libcerror_error_t **error )
{
	libfsxfs_extent_t *extent                                           = NULL;
	libfsxfs_internal_extended_attribute_t *internal_extended_attribute = NULL;
	static char *function                                               = "libfsxfs_extended_attribute_get_extent_by_index";
	int result                                                          = 1;

	if( extended_attribute == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid extended attribute.",
		 function );

		return( -1 );
	}
	internal_extended_attribute = (libfsxfs_internal_extended_attribute_t *) extended_attribute;

#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_grab_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to grab read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	if( libfsxfs_inode_get_attributes_extent_by_index(
	     internal_extended_attribute->inode,
	     extent_index,
	     &extent,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve attribute extent: %d.",
		 function,
		 extent_index );

		result = -1;
	}
	if( result == 1 )
	{
		if( libfsxfs_extent_get_values(
		     extent,
		     internal_extended_attribute->io_handle,
		     extent_offset,
		     extent_size,
		     extent_flags,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve extent: %d values.",
			 function,
			 extent_index );

			result = -1;
		}
	}
#if defined( HAVE_LIBFSXFS_MULTI_THREAD_SUPPORT )
	if( libcthreads_read_write_lock_release_for_write(
	     internal_extended_attribute->read_write_lock,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to release read/write lock for writing.",
		 function );

		return( -1 );
	}
#endif
	return( result );
}

