package edu.duke.raft;

import java.util.LinkedList;

public class RaftLog {
  public RaftLog (String file) {
  }

  // @return highest index of appended entries on success, 0 otherwise
  public int append (Entry[] entry){
    return 0;
  }

  // @return index of last entry in log
  public int getLastIndex () {
    return 0;
  }

  // @return term of last entry in log
  public int getLastTerm () {
    return 0;
  }

  // @return entry at passed-index, null if none
  public Entry getEntry (int index) {
    return null;
  }

  // @return highest index of appended entries on success, 0 otherwise  
  public int insertAt (Entry entry, int index) {
    return 0;
  } 

  private void init () {
  }
  
}
