// socket.cc - source file for the mailfilter program
// Copyright (c) 2003 - 2009  Andreas Bauer <baueran@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.
//
// Note that this program is released under the GPL with the additional
// exemption  that compiling, linking, and/or using OpenSSL is allowed.

#include <iostream>
#include <csignal>
#include <cstring>
#include <string>
#include <stdexcept>
#include <sstream>

#ifdef USE_SSL
extern "C"
{
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
}
#endif

extern "C"
{
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
}

#include "defines.hh"
#include "mailfilter.hh"
#include "feedback.hh"
#include "socket.hh"
#include "protocol.hh"

using namespace std;

static sigjmp_buf curr_env;

// TODO: these variables should not "just" be global; maybe static members or something?
#ifdef USE_SSL
  SSL*        ssl;
  BIO*        sbio;
  SSL_METHOD* ssl_meth;
  SSL_CTX*    ssl_ctx;
#endif

Socket :: Socket (void)
{ set_ssl (false); }

void Socket :: clear (void)
{
  // TODO: the following crashes the program at the end; why?
  // if (read_buffer)
  //   delete read_buffer;
}

int Socket :: c_open (const char* host_name,
		      int port,
		      int new_time_out,
		      int the_protocol)
{
  struct sockaddr_in  s_address;
  struct hostent      *host;
  struct sigaction    sig_action;

  Feedback* logger = Feedback :: Instance ();

  // Install alarm signal handler for interrupts and connnection
  // time out handling.
  time_out = new_time_out;
  sig_action.sa_handler = connect_alarm;
  sigemptyset (&sig_action.sa_mask);
  sig_action.sa_flags = 0;

  // Try to initialise signal handler and save stack context.
  if (sigaction (SIGALRM, &sig_action, NULL) < 0)
    {
      logger->print_err ("Installation of signal handler failed.");
      return GEN_FAILURE_FLAG;
    }
    
  if (sigsetjmp (curr_env, 1) == 0)
    {
      // Set time out.
      alarm (time_out);
  
#ifdef USE_SSL
      // Should SSL encryption be desired, establish a CTX object first.
      if (the_protocol == (PROTOCOL_POP3 | SSL_C)
	  || the_protocol == (PROTOCOL_APOP | SSL_C))
	{
	  logger->print_msg ("Debugging: Using SSL encrypted communication.", 6);
	  set_ssl (true);
	  SSL_library_init ();
	  SSL_load_error_strings();
	  
	  // Here should be some key verification stuff...
	  
	  ssl_meth = (SSL_METHOD*)(SSLv23_client_method ());
	  ssl_ctx  = (SSL_CTX*)(SSL_CTX_new (ssl_meth));
	}
      else
	logger->print_msg ("Debugging: Client-server communication is not encrypted.", 6);
#endif
      
      // Try to establish a socket communication endpoint and then
      // try to look up the hostname.
      if (! host_name
	  || ((sd = socket (PF_INET, SOCK_STREAM, 0)) < 0)
	  || (!(host = gethostbyname (host_name))))
	{
	  logger->print_err ("DNS look up error; is the hostname correct?");
	  alarm (0);
	  return GEN_FAILURE_FLAG;
	}
      
      memset ((char*) &s_address, 0, sizeof (struct sockaddr_in));
      s_address.sin_family = AF_INET;
      memcpy (&(s_address.sin_addr.s_addr),
	      host->h_addr,
	      host->h_length);
      s_address.sin_port = htons (port);
      
      if (connect (sd, (struct sockaddr*) &s_address, sizeof (struct sockaddr)) < 0)
	{
	  logger->print_err ("Server connection could not be established.");
	  alarm (0);
	  return GEN_FAILURE_FLAG;
	}
      
#ifdef USE_SSL
      // Now, we have a TCP connection; start SSL negotiation, if desired.
      if (use_ssl ())
	{
	  if ((ssl = SSL_new (ssl_ctx)) == NULL)
	    {
	      logger->print_err ("SSL connection structure could not be created.");
	      exit (-1);
	    }
	  
	  // Either the following line, or BIO connection object?
	  // SSL_set_fd (ssl, sd);
	  
	  // Create BIO object and attach ssl object to it
	  if ((sbio = BIO_new_socket (sd, BIO_NOCLOSE)) == NULL)
	    {
	      logger->print_err ("Socket-BIO could not be created.");
	      return GEN_FAILURE_FLAG;
	    }
	  SSL_set_bio (ssl, sbio, sbio);
	  
	  if (SSL_connect (ssl) <= 0)
	    {
	      logger->print_err ("SSL connection could not be established.");
	      return GEN_FAILURE_FLAG;
	    }
	}
#endif
    }
  else
    {
      logger->print_err ("Timeout occured.");
      return GEN_FAILURE_FLAG;
    }

  // Reset alarm again.
  alarm (0);
  return 0;
}

// Like close(2), this function returns 0 on success, and -1
// otherwise.  Its purpose is to close the socket connection.

int Socket :: c_close (void) const
{ 
#ifdef USE_SSL
  if (use_ssl ())
    {
      SSL_shutdown (ssl);
      int close_err = close (sd);
      SSL_free (ssl);
      SSL_CTX_free (ssl_ctx);
      return close_err;
    }
  else
    return close (sd);
#else
  return close (sd);
#endif
}

int Socket :: c_write (const char* msg)
{
  if (msg)
    {
      Feedback* logger = Feedback :: Instance ();

      string tmp_msg = msg;
      ssize_t bytes;

#ifdef USE_SSL
      if (use_ssl ())
	bytes = SSL_write (ssl, msg, strlen (msg));
      else
	bytes = write (sd, msg, strlen (msg));
#else
      bytes = write (sd, msg, strlen (msg));
#endif

      if (bytes < 0)
	logger->print_err (strerror (errno));

      // Debugging.
      if (tmp_msg.find ("PASS ") != 0)
	logger->print_msg ("Debugging: Wrote: " + tmp_msg, 6);
      else
	logger->print_msg ("Debugging: Wrote: PASS *****", 6);

      return bytes;
    }
  return GEN_FAILURE_FLAG;
}

// This function can be used to receive a server's response via the
// socket connection.  It returns the number of bytes read upon
// success, and a negative integer otherwise.  The reply string itself
// can then be obtained by calling the member function c_reply ().  If
// read_header is true, the c_read function will not stop reading when
// the host stops transmitting data.  Instead, it expects an entire
// header to be sent and thus, will receive data until \r\n.\r\n is
// encountered, i.e. the end of the header according to RFC822.

int Socket :: c_read (bool read_header)
{
  Feedback* logger = Feedback :: Instance ();

  char buffer[MAX_BYTES + 10];
  struct timeval tv;
  string input;
  int flags;
  int prem_exit = 0;
  int bytes;
  int counter;
  int error;
  fd_set rfds;

  // Empty the current read_buffer which may contain previous server
  // responses.
  // if (read_buffer)
  // delete read_buffer;

  // First, try to read the current socket descriptor's flags and
  // store a copy in 'flags', and then change the communication of
  // that descriptor to nonblocking I/O.
  if (((flags = fcntl (sd, F_GETFL, 0)) == -1)
      || (fcntl (sd, F_SETFL, flags | O_NONBLOCK) == -1))
    {
      logger->print_err (strerror (errno));
      return GEN_FAILURE_FLAG;
    }
  
  // Set connection time out.
  tv.tv_sec = time_out;
  tv.tv_usec = 0;
  FD_ZERO (&rfds);
  FD_SET (sd, &rfds);
  
  // Only read if the socket is ready and able to send data.
  error = select (sd + 1, &rfds, NULL, NULL, &tv);
  
  // An error occured while trying to poll server.  We did not get a
  // file descriptor.
  if (error < 0)
    {
      logger->print_err (strerror (errno));
      return GEN_FAILURE_FLAG;
    }
  // One (or many) file descriptors are ready for message exchange.
  else if (error > 0)
    {
      // Check the status of the file descriptor.
      if (!FD_ISSET (sd, &rfds))
	return GEN_FAILURE_FLAG;

      // Start receiving messages from the server.
      do
	{
	  memset (buffer, 0, sizeof buffer);

#ifdef USE_SSL
	  if (use_ssl ())
	    {
	      if (ssl)
		{
		  bytes = SSL_read (ssl, buffer, MAX_BYTES);
		  int i = SSL_get_error (ssl, bytes);

		  switch (i)
		    {
		    case SSL_ERROR_NONE:
		      prem_exit = 0;
		      break;
		    case SSL_ERROR_WANT_READ:
		      prem_exit = 1;
		      logger->print_msg ("Debugging: Warning: Received SSL_ERROR_WANT_READ.", 6);
		      break;
		    case SSL_ERROR_WANT_WRITE:
		      prem_exit = 1;
		      logger->print_msg ("Debugging: Warning: Received SSL_ERROR_WANT_WRITE.", 6);
		      break;
		    case SSL_ERROR_ZERO_RETURN:
		      prem_exit = 1;
		      logger->print_msg ("Debugging: Warning: Received SSL_ERROR_ZERO_RETURN.", 6);
		      break;
		    default:
		      logger->print_err ("SSL_read returned unknown error.");
		      return GEN_FAILURE_FLAG;
		    }
		}
	      else
		{
		  logger->print_err ("For unknown reason the SSL connection was closed.");
		  return GEN_FAILURE_FLAG;
		}
	    }
	  else
	    bytes = recv (sd, buffer, MAX_BYTES, 0);
#else
	  bytes = recv (sd, buffer, MAX_BYTES, 0);
#endif
	  
	  if (bytes > 0)
	    {
	      counter += bytes;
	      
	      if (input.length ())
		input.append (buffer, bytes);
	      else
		{
 		  input = buffer;
		  input += "\0";
		}
	    }

	  // The end condition for reading is as follows: keep reading
	  // as long as there was something to read previously, or, if
	  // there hasn't been (and we've surpassed the MTU), then see
	  // whether we encountered the end of the message header yet,
	  // which has to be ".\r\n".
	} while (bytes > 0
		 || (read_header
		     // The following finishes a POP3 server reply.
		     && ( (input[input.length () - 1] != '\n'
			   || input[input.length () - 2] != '\r'
			   || input[input.length () - 3] != '.')
			  // The following finishes an IMAP server reply.
			  && input.find (")\r\na OK FETCH completed\r\n")
			                                == string :: npos )
		     ));
    }
  else
    {
      logger->print_err ("Connection has timed out.");
      return GEN_FAILURE_FLAG;
    }

  // Put line ending behind the buffer, if needed.  Do not use '\0'
  // instead of "\0" because, somehow this crashes the string append
  // function!  (Why?  If it's illegal, the type system should
  // choke...)  Alternatively, one could probably write
  // input[counter] = '\0';
  // TODO: Verify that and chose best code!
  if (input.length () 
      && input.find_last_of ('\0', input.length ()) == string :: npos)
    input.append ("\0");

  // Debugging.
  logger->print_msg ("Debugging: Read: " + 
		    (input[input.length () - 1] != '\n'? input + "\n" : input),
		    6);
  
  // Reset socket communication to good old blocking mode.
  if ((fcntl (sd, F_SETFL, flags)) == -1)
    {
      logger->print_err (strerror (errno));
      return GEN_FAILURE_FLAG;
    }
  
  // Return the length of the server response and catch out of memory
  // exceptions.  I do not want to pass the exception though, in order
  // to keep the calling code simple.  That is, in case of the memory
  // error, MEM_FAILURE_FLAG is returned.  This could be used to react
  // especially to this case, if necessary at all.  Most likely, the
  // calling code is only interested in the distinction between a
  // positive and a negative return value anway; the failure flags
  // are, of course, both negative.
  try
    {
      read_buffer = new string (input);
      return read_buffer->length ();
    }
  catch (const exception& r_err)
    {
      logger->print_err (r_err.what ());
      return MEM_FAILURE_FLAG;
    }
}

const string* Socket :: c_reply (void) const
{  return read_buffer;  }

// Alarm signal call back function.

void Socket :: connect_alarm (int signo)
{
  // No need to explicitly mask signals in a one-liner,
  // is there?
  siglongjmp (curr_env, signo);
}

bool Socket :: use_ssl (void) const
{ return ssl_used; }

void Socket :: set_ssl (bool the_ssl)
{ ssl_used = the_ssl; }
