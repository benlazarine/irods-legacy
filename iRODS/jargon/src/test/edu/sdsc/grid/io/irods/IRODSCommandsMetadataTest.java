package edu.sdsc.grid.io.irods;

import edu.sdsc.grid.io.MetaDataRecordList;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

import org.junit.After;
import org.junit.AfterClass;

import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static org.junit.Assert.*;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.util.Properties;


/**
 * Tests for icommands that query metadata
 * @author Mike Conway, DICE (www.irods.org)
 * @since
 *
 */
public class IRODSCommandsMetadataTest {
    private static Properties testingProperties = new Properties();
    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
    private static ScratchFileUtils scratchFileUtils = null;
    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSCommandsMetadataTest";
    private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
        testingProperties = testingPropertiesLoader.getTestProperties();
        scratchFileUtils = new ScratchFileUtils(testingProperties);
        scratchFileUtils.createDirectoryUnderScratch(IRODS_TEST_SUBDIR_PATH);
        irodsTestSetupUtilities = new IRODSTestSetupUtilities();
        irodsTestSetupUtilities.initializeIrodsScratchDirectory();
        irodsTestSetupUtilities.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
    }

    @AfterClass
    public static void tearDownAfterClass() throws Exception {
    }

    @Before
    public void setUp() throws Exception {
    }

    @After
    public void tearDown() throws Exception {
    }

    /**
     * test relevant to BUG: 24 Inconsistant return of resource
     * @throws Exception
     */
    @Test
    public final void testQueryForResourceViaIRODSFile() throws Exception {
    	
    	// Aan case 2
    	 IRODSAccount account = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
         IRODSFileSystem irodsFileSystem = new IRODSFileSystem(account);
         String testFileName = "testQueryForResource.txt";
         
         // generate a file and put into irods
         String fullPathToTestFile = FileGenerator.generateFileOfFixedLengthGivenName(testingProperties.getProperty(
                 GENERATED_FILE_DIRECTORY_KEY) + IRODS_TEST_SUBDIR_PATH + "/", testFileName, 1);
         
         IputCommand iputCommand = new IputCommand();
         iputCommand.setLocalFileName(fullPathToTestFile);
         iputCommand.setIrodsFileName(testingPropertiesHelper.buildIRODSCollectionRelativePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH ));
         iputCommand.setForceOverride(true);

         IrodsInvocationContext invocationContext = testingPropertiesHelper.buildIRODSInvocationContextFromTestProperties(testingProperties);
         IcommandInvoker invoker = new IcommandInvoker(invocationContext);
         invoker.invokeCommandAndGetResultAsString(iputCommand);
         
         IRODSFile irodsFile = new IRODSFile(irodsFileSystem, testingPropertiesHelper.buildIRODSCollectionAbsolutePathFromTestProperties(testingProperties, IRODS_TEST_SUBDIR_PATH));
           
         String filename = irodsFile.getName();
         long size = irodsFile.length();

         // query(String[]) defined in GeneralFile
         // will call query(String[] conditions, String[] selects) in IRODSFile, with null in conditions and these 
         // values in selects.  The values in the selects are abstract data names that are translated
         // into IRODS-specific query symbols in IRODSMetaDataSet
         
         // trickles down to IRODSCommands.query()
         
         MetaDataRecordList[] lists = irodsFile.query(new String[] {
                 IRODSMetaDataSet.RESOURCE_NAME,
                 IRODSMetaDataSet.RESOURCE_LOCATION });
        
         for (MetaDataRecordList l : lists) {
             String resource = l.getStringValue(0);
             String physicalResource = l.getStringValue(1);
         }
         
         irodsFileSystem.close();    
         
         // FIXME: add asserts
    }    
}