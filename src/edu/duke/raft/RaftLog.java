package edu.duke.raft;

import java.io.IOException;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.channels.Channels;
import java.nio.file.Files;
import java.nio.file.FileSystems;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.List;
import java.util.LinkedList;

public class RaftLog {
  private LinkedList<Entry> mEntries;
  private Path mLogPath;
  
  public RaftLog (String file) {
    mEntries = new LinkedList<Entry> ();
    try {
      mLogPath = FileSystems.getDefault().getPath (file);
      String delims = " ";
      List<String> lines = Files.readAllLines (mLogPath, 
					       StandardCharsets.US_ASCII);
      Entry e = null;
      
      for (String line : lines) {
	String[] tokens = line.split (delims);
	if ((tokens != null) && (tokens.length > 0)) {
	  e = new Entry (Integer.parseInt (tokens[1]),
			 Integer.parseInt (tokens[0]));
	  mEntries.add (e);
	} else {
	  System.out.println ("Error parsing log line: " + line);
	}	
      }      
    } catch (IOException e) {
      System.out.println (e.getMessage ());
    }    
  }

  // @return highest index in log after entries have been appended
  public int append (Entry[] entries) {
    try {
      OutputStream out = Files.newOutputStream (mLogPath, 
						StandardOpenOption.APPEND,
						StandardOpenOption.SYNC);
      for (Entry entry : entries) {
	out.write (entry.toString ().getBytes ());
	out.write ('\n');
	mEntries.add (entry);
      }      
      out.close ();
    } catch (IOException e) {
      System.out.println (e.getMessage ());
    }
     
    return mEntries.size ();
  }

  // @return index of last entry in log
  public int getLastIndex () {
    return mEntries.size ();
  }

  // @return term of last entry in log
  public int getLastTerm () {
    Entry entry = mEntries.getLast ();
    if (entry != null) {
      return entry.term;
    }
    return -1;
  }

  // @return entry at passed-index, null if none
  public Entry getEntry (int index) {
    return null;
  }

  // @return highest index of appended entries on success, 0 otherwise  
  public int insertAt (Entry entry, int index) {
    return 0;
  } 

  public String toString () {
    String toReturn = "{";
    for (Entry e: mEntries) {
      toReturn += " (" + e + ") ";
    }
    toReturn += "}";
    return toReturn;
  }  

  private void init () {
  }

  public static void main (String[] args) {
    if (args.length != 1) {
      System.out.println("usage: java edu.duke.raft.RaftLog <filename>");
      System.exit(1);
    }
    String filename = args[0];
    RaftLog log = new RaftLog (filename);
    System.out.println ("RaftLog: " + log);

    Entry[] entries = new Entry[1];
    entries[0] = new Entry (0, 0);
    log.append (entries);
  }

}
