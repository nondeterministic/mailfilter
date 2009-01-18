// weeder.cc - source file for the mailfilter program
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

#include <vector>
#include <string>
#include <iostream>
#include <cstring>
#include "weeder.hh"
#include "header.hh"
#include "preferences.hh"
#include "mailfilter.hh"
#include "feedback.hh"
#include "defines.hh"

using namespace std;

extern string int_to_string (int);

// The function takes a pointer to a Header class which contains a
// message header.  It then determines whether this header is spam, a
// duplicate, or "good" mail.  It returns 1 if the mail qualifies as
// spam, 0 if it should not be deleted, and a negative integer upon
// error.

int Weeder :: is_weed (Header* the_header)
{
  int status;

  status = check_duplicates (the_header);
  if (status == 1)
    return 1;             // Spam.
  else if (status < 0)
    return status;        // Error.

  status = check_allow_rules (the_header);
  if (status == 1)
    return 1;             // Spam.
  else if (status == 0)
    return 0;             // Friend.
  else if (status < 0)
    return status;        // Error.

  status = check_maxlength (the_header);
  if (status == 1)
    return 1;             // Spam.

  status = check_deny_rules (the_header);
  if (status == 1)
    return 1;             // Spam.
  else if (status < 0)
    return status;        // Error.

  status = check_scores (the_header);
  if (status == 1)
    return 1;             // Spam.

  return 0;               // Leave message alone.
}

// This function returns 1 if the message was considered being a
// duplicate, and 0 otherwise.  A negative integer is returned upon
// error.

int Weeder :: check_duplicates (Header* the_header)
{
  Feedback* logger = Feedback :: Instance ();

  if (!the_header)
    return GEN_FAILURE_FLAG;
  
  // If the mailer didn't attach a valid message-ID, give up scanning
  // for duplicated messages immediately.
  if (the_header->ID ()->length() <= 0)
    return 0;

  string cur_line;
  
  if (Preferences :: Instance ().delete_duplicates ())
    {
      for (vector<string> :: iterator cur_ID = msg_ids.begin ();
	   cur_ID != msg_ids.end ();
	   cur_ID++)
	if (*(the_header->ID ()) == *cur_ID)
	  {
	    logger->print_msg ("Deny: "
			       + *(the_header->from ()) + ": "
			       + *(the_header->subject ()) + ", "
			       + *(the_header->date ())
			       + " [Duplicate].",
			       2);
	    return 1;
	  }
      msg_ids.push_back (*(the_header->ID ()));
    }

  return 0;
}

// This function returns 1 if the message was considered being spam, 0
// if it is a friend.  A negative integer is returned upon error.

int Weeder :: check_allow_rules (Header* the_header) const
{
  Feedback* logger = Feedback :: Instance ();

  if (!the_header)
    return GEN_FAILURE_FLAG;
  
  string cur_line;
  
  for (vector<entry> :: iterator cur_entry = the_header->entries ()->begin ();
       cur_entry != the_header->entries ()->end ();
       cur_entry++)
    {
      cur_line = cur_entry->tag + ": " + cur_entry->body;
      for (vector<Filter> :: iterator cur_allow =
	     Preferences :: Instance ().allow_filters ()->begin ();
	   cur_allow != Preferences :: Instance ().allow_filters ()->end ();
	   cur_allow++)
	{
	  if (!cur_allow->is_negative ()
	      && regexec (cur_allow->comp_exp (),
			  cur_line.c_str (),
			  0, 
			  NULL,
			  0) 
	      == 0)
	    {
	      logger->print_msg ("Allow: "
				 + *(the_header->from ()) + ": "
				 + *(the_header->subject ()) + ", "
				 + *(the_header->date ())
				 + " [\"" 
				 + cur_allow->expression ()
				 + "\" matches \""
				 + cur_line + "\"].",
				 4);
	      
	      // OK, friendly message detected.  Now check, whether
	      // MAXSIZE_ALLOW applies.
	      if (Preferences :: Instance ().max_size_allow () > 0
		  && the_header->size () > Preferences :: Instance ().max_size_allow ())
		{
		  logger->print_msg ("Deny: "
				     + *(the_header->from ()) + ": "
				     + *(the_header->subject ()) + ", "
				     + *(the_header->date ())
				     + " [Maxsize_Allow exceeded].",
				     2);
		  return 1;
		}

	      return 0;
	    }
	}
    }

  // Evaluate the negative allow rules which works slightly different
  // to the above algorithm: instead of going through each line and
  // each filter, we go through each negative filter and then through
  // all lines of the header.
  if (Preferences :: Instance ().neg_allows () > 0)
    {
      for (vector<Filter> :: iterator cur_allow =
	     Preferences :: Instance ().allow_filters ()->begin ();
	   cur_allow != Preferences :: Instance ().allow_filters ()->end ();
	   cur_allow++)
	{
	  if (cur_allow->is_negative ())
	    {
	      for (vector<entry> :: iterator cur_entry =
		     the_header->entries ()->begin ();
		   cur_entry != the_header->entries ()->end ();
		   cur_entry++)
		{
		  cur_line = cur_entry->tag + ": " + cur_entry->body;
		  if (regexec (cur_allow->comp_exp (),
			       cur_line.c_str (),
			       0, 
			       NULL,
			       0) 
		      == 0)
		    break;
		  
		  if (cur_entry + 1 == the_header->entries ()->end ())
		    {
		      logger->print_msg ("Allow: "
					 + *(the_header->from ()) + ": "
					 + *(the_header->subject ()) + ", "
					 + *(the_header->date ())
					 + " [Negative allow rule \""
					 + cur_allow->expression ()
					 + "\" did not match].",
					 4);
		      return 0;
		    }
		}
	    }
	}
    }
  
  return 2;
}

int Weeder :: check_maxlength (Header* the_header) const
{
  Feedback* logger = Feedback :: Instance ();

  string cur_line;

  if (Preferences :: Instance ().maxlength () == 0)
    return 0;

  for (vector<entry> :: iterator cur_entry = the_header->entries ()->begin ();
       cur_entry != the_header->entries ()->end ();
       cur_entry++)
    {
      cur_line = cur_entry->tag + ": " + cur_entry->body;
      
      if (cur_line.length () > (unsigned int)Preferences :: Instance ().maxlength ())
	{
	  logger->print_msg ("Deny: "
			    + *(the_header->from ()) + ": "
			    + *(the_header->subject ()) + ", "
			    + *(the_header->date ())
			    + " [\""
			    + cur_entry->tag
			    + "\" exceeded maxlength].",
			    2);
	  return 1;
	}
    }
  
  return 0;
}

// This function returns 1 if the message was considered being spam, 0
// otherwise.  A negative integer is returned upon error.

int Weeder :: check_deny_rules (Header* the_header) const
{
  Feedback* logger = Feedback :: Instance ();

  if (!the_header)
    return GEN_FAILURE_FLAG;
  
  string cur_line;

  if (Preferences :: Instance ().max_size_deny () > 0
      && the_header->size () > Preferences :: Instance ().max_size_deny ())
    {
      logger->print_msg ("Deny: "
			 + *(the_header->from ()) + ": "
			 + *(the_header->subject ()) + ", "
			 + *(the_header->date ())
			 + " [Maxsize_Deny exceeded].",
			 2);
      return 1;
    }
  for (vector<entry> :: iterator cur_entry = the_header->entries ()->begin ();
       cur_entry != the_header->entries ()->end ();
       cur_entry++)
    {
      cur_line = cur_entry->tag + ": " + cur_entry->body;
      for (vector<Filter> :: iterator
	     cur_deny = Preferences :: Instance ().deny_filters ()->begin ();
	   cur_deny != Preferences :: Instance ().deny_filters ()->end ();
	   cur_deny++)
	{
	  if (!cur_deny->is_negative ()
	      && regexec (cur_deny->comp_exp (),
			  cur_line.c_str (),
			  0, 
			  NULL,
			  0) 
	      == 0)
	    {
	      logger->print_msg ("Deny: "
				+ *(the_header->from ()) + ": "
				+ *(the_header->subject ()) + ", "
				+ *(the_header->date ())
				+ " [\"" 
				+ cur_deny->expression () +
				+ "\" matches \""
				+ cur_line + "\"].",
				2);
	      return 1;
	    }
	  
	  // Check normalised subject, if necessary.
	  if (Preferences :: Instance ().normal ()
	      && !cur_deny->is_negative ()
	      && strcmp (cur_entry->tag.c_str (), "Subject") == 0
	      && regexec (cur_deny->comp_exp (),
			  (the_header->normal_subject ())->c_str (),
			  0, NULL, 0)
	      == 0)
	    {
	      logger->print_msg ("Deny: "
				 + *(the_header->from ()) + ": "
				 + *(the_header->subject ()) + ", "
				 + *(the_header->date ())
				 + " [\"" 
				 + cur_deny->expression () +
				 + "\" matches \""
				 + cur_line 
				 + "\" (normalised)].",
				 2);
	      return 1;
	    }
	}
    }
  
  // Evaluate the negative deny rules in a similar fashion as negative
  // allow rules (see above, for comments).
  if (Preferences :: Instance ().neg_denies () > 0)
    {
      for (vector<Filter> :: iterator cur_deny =
	     Preferences :: Instance ().deny_filters ()->begin ();
	   cur_deny != Preferences :: Instance ().deny_filters ()->end ();
	   cur_deny++)
	{
	  if (cur_deny->is_negative ())
	    {
	      for (vector<entry> :: iterator cur_entry =
		     the_header->entries ()->begin ();
		   cur_entry != the_header->entries ()->end ();
		   cur_entry++)
		{
		  cur_line = cur_entry->tag + ": " + cur_entry->body;
		  if (regexec (cur_deny->comp_exp (),
			       cur_line.c_str (),
			       0, 
			       NULL,
			       0) 
		      == 0)
		    break;
		  
		  // Check for normalised subject, if applicable.
		  if (Preferences :: Instance ().normal ()
		      && strcmp (cur_entry->tag.c_str (), "Subject") == 0
		      && regexec (cur_deny->comp_exp (),
				  (the_header->normal_subject ())->c_str (),
				  0, NULL, 0)
		      == 0)
		    break;
		  
		  if (cur_entry + 1 == the_header->entries ()->end ())
		    {
		      logger->print_msg ("Deny: "
					 + *(the_header->from ()) + ": "
					 + *(the_header->subject ()) + ", "
					 + *(the_header->date ())
					 + " [Negative deny rule \""
					 + cur_deny->expression ()
					 + "\" did not match].",
					 4);
		      return 1;
		    }
		}
	    }
	}
    }

  return 0;
}

// This function returns the score the message achieved.

int Weeder :: check_scores (Header* the_header) const
{
  Feedback* logger = Feedback :: Instance ();

  string cur_line;
  int msg_score = 0;

  // First test against the MAXSIZE_SCORE setting
  if (Preferences :: Instance ().max_size_score().score != 0
      && Preferences :: Instance ().max_size_score().size > 0
      && the_header->size() > Preferences :: Instance ().max_size_score().size)
    msg_score = Preferences :: Instance ().max_size_score().score;

  // Now check ordinary score rules.
  for (vector<entry> :: iterator cur_entry = the_header->entries ()->begin ();
       cur_entry != the_header->entries ()->end ();
       cur_entry++)
    {
      cur_line = cur_entry->tag + ": " + cur_entry->body;

      for (vector<Score> :: iterator cur_score = 
	     Preferences :: Instance ().score_filters ()->begin ();
	   cur_score != Preferences :: Instance ().score_filters ()->end ();
	   cur_score++)
	{
	  if (!cur_score->is_negative ()
	      // Check ordinary score rules...
	      && ((regexec (cur_score->comp_exp (),
			    cur_line.c_str (),
			    0, 
			    NULL,
			    0) == 0)
		  || // ...or, if that doesn't match, ...
		  // ...check normalised subject, if applicable.
		  (Preferences :: Instance ().normal ()
		   && strcmp (cur_entry->tag.c_str (), "Subject") == 0
		   && regexec (cur_score->comp_exp (),
			       (the_header->normal_subject ())->c_str (),
			       0, NULL, 0) == 0)))
	    {
	      msg_score += cur_score->score ();
	      logger->print_msg ("Score: \""
				 + cur_score->expression ()
				 + "\" matches \""
				 + cur_line
				 + "\" ["
				 + int_to_string (cur_score->score ())
				 + "].",
				 5);
	    }
	}
    }

  // Check negative scores now.
  for (vector<Score> :: iterator cur_score = 
	 Preferences :: Instance ().score_filters ()->begin ();
       cur_score != Preferences :: Instance ().score_filters ()->end ();
       cur_score++)
    {
      if (cur_score->is_negative ())
	{
	  for (vector<entry> :: iterator 
		 cur_entry = the_header->entries ()->begin ();
	       cur_entry != the_header->entries ()->end ();
	       cur_entry++)
	    {
	      cur_line = cur_entry->tag + ": " + cur_entry->body;
	      
	      if (regexec (cur_score->comp_exp (),
			   cur_line.c_str (),
			   0, 
			   NULL,
			   0) == 0)
		break;
	      
	      // Check normalised subject, if applicable.
	      if (Preferences :: Instance ().normal ()
		  && strcmp (cur_entry->tag.c_str (), "Subject") == 0
		  && regexec (cur_score->comp_exp (),
			      (the_header->normal_subject ())->c_str (),
			      0, NULL, 0) == 0)
		break;
	      
	      if (cur_entry + 1 == the_header->entries ()->end ())
		{
		  msg_score += cur_score->score ();
		  logger->print_msg ("Score: <> \""
				     + cur_score->expression ()
				     + "\" did not match "
				     + "["
				     +  int_to_string (cur_score->score ())
				     + "].",
				     5);
		}
	    }
	}
    }

  if (msg_score >= Preferences :: Instance ().highscore ())
    {
      logger->print_msg ("Deny: "
			 + *(the_header->from ()) + ": "
			 + *(the_header->subject ()) + ", "
			 + *(the_header->date ())
			 + " [Score: "
			 + int_to_string (msg_score)
			 + "].",
			 2);
      return 1;
    }
  
  logger->print_msg ("Pass: "
		     + *(the_header->from ()) + ": "
		     + *(the_header->subject ()) + ", "
		     + *(the_header->date ())
		     + " [Score: "
		     + int_to_string (msg_score)
		     + "].",
		     5);
  return 0;
}
