package edu.duke.raft;
import java.util.*;

// Every server starts as a follower. 
// Objectives:
// 			1. Detect when the current leader has failed. /Detect failure by setting "election timeout" (random value between 150 - 300 ms. 
//			FAILED-->CANDIDATE MODE
//			2. Vote in election (implement the refined election criterion)


public class FollowerMode extends RaftMode {
	private int fTitle = 2;
	private Timer timer;
	private Random random = new Random();
	private boolean isSwitched = true; 
	
	private void followerTimer(){
		synchronized(mLock){
  			if(mConfig.getTimeoutOverride()<=0){
	 			timer = scheduleTimer((long) random.nextInt(ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN + 1) + (ELECTION_TIMEOUT_MIN), fTitle);
  			} else{
  				timer = scheduleTimer)mConfig.getTimeoutOverride(), fTitle);
  			}
		}
	}
  


  public void go () {
    synchronized (mLock) {
      int term = 0;
      System.out.println ("S" + 
			  mID + 
			  "." + 
			  term + 
			  ": switched to follower mode.");
	  isSwitched = false;
	  followerTimer();
    }
  }
  
  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      if(isSwitched || candidateTerm<term) return term;
      if(candidateTerm>term){
      	if(lastLogTerm>mLog.getLastTerm() || (lastLogIndex >= mLog.getLastIndex() && lastLogTerm==mLog.getLastTerm())) {
      		timer.cancel();
      		mConfig.setCurrentTerm(candidateTerm,candidateID);
      		followerTimer();
      		return 0;
      	} else {
      		if(mConfig.getVotedfor()==0||mConfig.getVotedfor()==candidateID){
      			if(lastLogTerm>mLog.getLastTerm() || (lastLogIndex >= mLog.getLastIndex() && lastLogTerm==mLog.getLastTerm())){
      				timer.cancel();
      				mConfig.setCurrentTerm(candidateTerm,candidateID);
      				followerTimer();
      				return 0;
      			}
      		}
      	} else{
      		return mConfig.getCurrentTerm();	
      	}
      }
    }
  }
  

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      if(isSwitched || leaderTerm<term) return term;
      if(leaderTerm>term) mConfig.setCurrentTerm(leaderTerm,0);
      timer.cancel();
      followerTimer();
      //NOt complete
    }
  }  

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
    	
    }
  }
}

