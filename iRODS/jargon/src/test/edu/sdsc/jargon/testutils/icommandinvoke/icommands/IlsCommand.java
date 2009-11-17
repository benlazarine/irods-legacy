/**
 *
 */
package edu.sdsc.jargon.testutils.icommandinvoke.icommands;

import java.util.ArrayList;
import java.util.List;

/**
 * @author Mike
 *
 */
public class IlsCommand implements Icommand {

	private String ilsBasePath = "";
	
	public String getIlsBasePath() {
		return ilsBasePath;
	}

	public void setIlsBasePath(String ilsBasePath) {
		this.ilsBasePath = ilsBasePath;
	}

	/* (non-Javadoc)
	 * @see org.irods.jargon.icommandinvoke.icommands.Icommand#buildCommand()
	 */
	@Override
	public List<String> buildCommand() {
		List<String> commandProps = new ArrayList<String>();
		commandProps.add("ils");
		if (ilsBasePath.length() > 0) {
			commandProps.add(ilsBasePath);
		}
		return commandProps;
	}

	

}
