/**
 *
 */
package edu.sdsc.jargon.testutils.icommandinvoke.icommands;

import edu.sdsc.jargon.testutils.icommandinvoke.IcommandException;

import java.util.ArrayList;
import java.util.List;


/**
 * Implement the irm irods icommand specifying an object and options
 * @author Mike Conway, DICE (www.irods.org)
 * @since 10/20/2009
 */
public class IrmCommand implements Icommand {
    private String objectName = "";
    private boolean force = false;
    private boolean verbose = false;

    public boolean isVerbose() {
		return verbose;
	}

	public void setVerbose(boolean verbose) {
		this.verbose = verbose;
	}

	public boolean isForce() {
        return force;
    }

    public void setForce(boolean force) {
        this.force = force;
    }
    
    public String getObjectName() {
        return objectName;
    }

    public void setObjectName(String objectName) {
        this.objectName = objectName;
    }

    /* (non-Javadoc)
     * @see org.irods.jargon.icommandinvoke.icommands.Icommand#buildCommand()
     */
    @Override
    public List<String> buildCommand() throws IcommandException {
        if ((objectName == null) || (objectName.length() == 0)) {
            throw new IllegalArgumentException("no object name specified");
        }

        List<String> commands = new ArrayList<String>();
        commands.add("irm");

        if (isForce()) {
            commands.add("-f");
        }
        
        if (isVerbose()) {
        	commands.add("-vv");
        }

        // TODO right now assume recursive...might need to be smarter about this
        commands.add("-r");
        commands.add(objectName);

        return commands;
    }

  
}
