package edu.sdsc.grid.io.irods;

import static org.junit.Assert.*;

import java.net.URI;
import java.util.Properties;

import junit.framework.TestCase;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import edu.sdsc.grid.io.irods.mocks.MockGssCredential;
import edu.sdsc.grid.io.local.LocalFile;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;

public class IRODSAccountTest {
	 private static Properties testingProperties = new Properties();
	    private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	    private static ScratchFileUtils scratchFileUtils = null;
	    public static final String IRODS_TEST_SUBDIR_PATH = "IRODSAccountTest";
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

	@Test
	public final void testIRODSAccount() throws Exception {
		// can I create an IRODSAccount on this platform based on data in my .irodsEnv?  OK if no exception
		IRODSAccount account = new IRODSAccount();
		
	}
	
	@Test
	public final void testIRODSAccountGeneralFile() throws Exception {
		LocalFile info = new LocalFile(System.getProperty("user.home")+"/.irods/");
	    if (!info.exists()) {
	      //Windows Scommands doesn't setup as "."
	      info = new LocalFile(System.getProperty("user.home")+"/irods/");
	    }
	    IRODSAccount irodsAccount = new IRODSAccount(info);
	}

	@Test
	public final void testIRODSAccountStringIntStringStringStringStringString() {
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), expectedIRODSAccount.getDefaultStorageResource());
		TestCase.assertEquals(expectedIRODSAccount.getDefaultStorageResource(), actualIRODSAccount.getDefaultStorageResource());
		TestCase.assertEquals(expectedIRODSAccount.getZone(), actualIRODSAccount.getZone());
		TestCase.assertEquals(expectedIRODSAccount.getHomeDirectory(), actualIRODSAccount.getHomeDirectory());
		TestCase.assertEquals(expectedIRODSAccount.getHost(), actualIRODSAccount.getHost());
		TestCase.assertEquals(expectedIRODSAccount.getPassword(), actualIRODSAccount.getPassword());
		TestCase.assertEquals(expectedIRODSAccount.getPort(), actualIRODSAccount.getPort());
		TestCase.assertEquals(expectedIRODSAccount.getUserName(), actualIRODSAccount.getUserName());		
	}
	
	@Test(expected=NullPointerException.class)
	public final void testIRODSAccountNoDefaultStorageResource() {
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), null);
		TestCase.assertEquals(expectedIRODSAccount.getDefaultStorageResource(), actualIRODSAccount.getDefaultStorageResource());
		TestCase.assertEquals(expectedIRODSAccount.getZone(), actualIRODSAccount.getZone());
		TestCase.assertEquals(expectedIRODSAccount.getHomeDirectory(), actualIRODSAccount.getHomeDirectory());
		TestCase.assertEquals(expectedIRODSAccount.getHost(), actualIRODSAccount.getHost());
		TestCase.assertEquals(expectedIRODSAccount.getPassword(), actualIRODSAccount.getPassword());
		TestCase.assertEquals(expectedIRODSAccount.getPort(), actualIRODSAccount.getPort());
		TestCase.assertEquals(expectedIRODSAccount.getUserName(), actualIRODSAccount.getUserName());		
	}

	@Test
	public final void testIRODSAccountStringIntGSSCredential() throws Exception {
		String host = testingProperties.getProperty(TestingPropertiesHelper.IRODS_HOST_KEY);
		int port = testingPropertiesHelper.getPortAsInt(testingProperties);
		MockGssCredential gssCredential = new MockGssCredential();
		IRODSAccount irodsAccount = new IRODSAccount(host, port, gssCredential);
		irodsAccount.setGSSCredential(gssCredential);
		TestCase.assertNotNull("irods account is null", irodsAccount);
		TestCase.assertEquals("host not found", host, irodsAccount.getHost());
		TestCase.assertEquals("port not found", port, irodsAccount.getPort());	
		TestCase.assertNotNull("gsi credential not set", irodsAccount.getGSSCredential());
	}

	@Ignore
	public final void testIRODSAccountStringIntGSSCredentialStringString() {
		fail("Not yet implemented");
	}

	@Test
	public final void testSetDefaultStorageResource() {
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), expectedIRODSAccount.getDefaultStorageResource());
		actualIRODSAccount.setDefaultStorageResource("hellothere");
	}

	@Test
	public final void testGetDefaultStorageResource() {
		String expectedDefaultResource = "hellothere";
		IRODSAccount expectedIRODSAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount actualIRODSAccount = new IRODSAccount(expectedIRODSAccount.getHost(), expectedIRODSAccount.getPort(),
				expectedIRODSAccount.getUserName(), expectedIRODSAccount.getPassword(), expectedIRODSAccount.getHomeDirectory(),
				expectedIRODSAccount.getZone(), expectedIRODSAccount.getDefaultStorageResource());
		actualIRODSAccount.setDefaultStorageResource(expectedDefaultResource);
		String actualDefaultResource = actualIRODSAccount.getDefaultStorageResource();
		TestCase.assertEquals(expectedDefaultResource, actualDefaultResource);
		
	}
	
	@Test
	public final void testSetCertificateAuthority() throws Exception {
		String expectedCA = "hows,that,for,a,test";
		IRODSAccount irodsAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		irodsAccount.setCertificateAuthority(expectedCA);
		TestCase.assertEquals("did not set the CA", expectedCA, irodsAccount.getCertificateAuthority());

	}
	
	@Test
	public final void testSetNullCertificateAuthority() throws Exception {
		IRODSAccount irodsAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		irodsAccount.setCertificateAuthority(null);
		TestCase.assertTrue(irodsAccount.getAuthenticationScheme().equals("PASSWORD"));
	}
	
	@Test
	public final void testEquals() throws Exception {
		IRODSAccount irodsAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSAccount otherAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		TestCase.assertTrue("equals() does not detect identical accounts", irodsAccount.equals(otherAccount));
	}
	
	@Test
	public final void testToURIWithPassword() throws Exception {
		IRODSAccount irodsAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		StringBuilder b = new StringBuilder();
		b.append("irods://");
		b.append(irodsAccount.getUserName());
		b.append(":");
		b.append(irodsAccount.getPassword());
		b.append("@");
		b.append(irodsAccount.getHost());
		b.append(":");
		b.append(irodsAccount.getPort());
		b.append(irodsAccount.getHomeDirectory());
		URI actualUri = irodsAccount.toURI(true);
		URI expectedUri = new URI(b.toString());
		TestCase.assertEquals("did not generate correct uri including password", expectedUri, actualUri);
		
	}
	
	@Test
	public final void testToURINoPassword() throws Exception {
		IRODSAccount irodsAccount = testingPropertiesHelper.buildIRODSAccountFromTestProperties(testingProperties);
		StringBuilder b = new StringBuilder();
		b.append("irods://");
		b.append(irodsAccount.getUserName());
		b.append("@");
		b.append(irodsAccount.getHost());
		b.append(":");
		b.append(irodsAccount.getPort());
		b.append(irodsAccount.getHomeDirectory());
		URI actualUri = irodsAccount.toURI(false);
		URI expectedUri = new URI(b.toString());
		TestCase.assertEquals("did not generate correct uri including password", expectedUri, actualUri);
		
	}
	
}
