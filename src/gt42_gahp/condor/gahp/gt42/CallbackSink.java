/*
 * CallbackSink.java
 *
 * Created on October 7, 2003, 4:01 PM
 */

package condor.gahp.gt42;

import condor.gahp.*;

import java.util.HashMap;
import java.util.Vector;
import java.util.List;
import java.util.LinkedList;
import java.util.Map;

import javax.xml.rpc.Stub;

import org.globus.exec.generated.FilePairType;
import org.globus.exec.generated.ManagedJobPortType;
import org.globus.exec.generated.StateChangeNotificationMessageType;
import org.globus.exec.generated.StateEnumeration;
import org.globus.exec.utils.ManagedJobConstants;
import org.globus.exec.utils.FaultUtils;

import org.globus.wsrf.WSNConstants;
import org.globus.wsrf.NotifyCallback;
import org.globus.wsrf.NotificationConsumerManager;
import org.globus.wsrf.encoding.ObjectSerializer;
import org.globus.wsrf.encoding.ObjectDeserializer;
import org.globus.wsrf.security.authorization.client.Authorization;
import org.globus.wsrf.impl.security.descriptor.GSITransportAuthMethod;
import org.globus.wsrf.impl.security.descriptor.ResourceSecurityDescriptor;
import org.globus.exec.generated.FaultResourcePropertyType;

import org.oasis.wsn.FilterType;
import org.oasis.wsrf.faults.BaseFaultType;
import org.oasis.wsrf.lifetime.Destroy;

import org.w3c.dom.Element;

import org.globus.exec.client.GramJob;
import org.globus.wsrf.container.ServiceContainer;
import org.globus.exec.utils.client.ManagedJobClientHelper;
import org.globus.exec.utils.NotificationUtil;

import org.globus.axis.message.addressing.EndpointReferenceType;

import org.oasis.wsn.SubscriptionManager;
import org.oasis.wsn.TopicExpressionType;
import org.oasis.wsn.WSBaseNotificationServiceAddressingLocator;
import org.oasis.wsn.Subscribe;
import org.oasis.wsn.SubscribeResponse;

import org.apache.axis.message.MessageElement;


/**
 *
 * @author  ckireyev
 *
 * This class is a wrapper around NotifyCallback - 
 * the GT42 way of receiving callbacks
 *
 */
public class CallbackSink
    implements CleanupStep, NotifyCallback
{
	

    private String callbackId;
    private int requestId;
    private GahpInterface gahp;
    private boolean isInitialized = false;
	private NotificationConsumerManager notificationConsumerManager;
	private Integer port = null;
    private EndpointReferenceType notificationConsumerEPR = null;
    
    public static final String CALLBACK_SINK = "CALLBACK_SINK";
	public static final String CALLBACK_SINK_LIST = "CALLBACK_SINK_LIST";
    
    /** Creates a new instance of CallbackSink */
    public CallbackSink (int requestId,
						 Integer port,
						 GahpInterface gahp) 
		throws Exception {
		this.requestId = requestId;
		this.gahp = gahp;
		this.port = port;

			// Start listening on a specified port...
		Map props = new HashMap();
		if (this.port != null) {
			props.put (ServiceContainer.PORT, 
					   this.port);
		}

        //make sure the embedded container speaks GSI Transport Security
        props.put(ServiceContainer.CLASS,
                  "org.globus.wsrf.container.GSIServiceContainer");

        // TODO Should we insert the currently active proxy in the
        //   properties? If we don't, does that mean no authentication
        //   is occurring? If we do, I think we need to refresh the
        //   credential when our invoker gives us a fresh one.

		this.notificationConsumerManager = 
			NotificationConsumerManager.getInstance(props);
		this.notificationConsumerManager.startListening();

			// Create a security descriptor
        // JEF This is copied from GramJob.setupNotificationConsumer()
		ResourceSecurityDescriptor securityDescriptor	
                = new ResourceSecurityDescriptor();
		securityDescriptor.setPDP(Authorization.AUTHZ_NONE);
		Vector authMethod = new Vector();
        authMethod.add(GSITransportAuthMethod.BOTH);
		securityDescriptor.setDefaultAuthMethods(authMethod);

		List topicPath = new LinkedList(); 	
		topicPath.add(ManagedJobConstants.STATE_CHANGE_INFORMATION_TOPIC_QNAME);
		notificationConsumerEPR = 
			notificationConsumerManager.createNotificationConsumer(
															topicPath,
															this,
															securityDescriptor);

		isInitialized = true;
	}

	public void addJobListener (GramJob job, String jobId) 
		throws Exception 
	{
		ManagedJobPortType jobPort = 
			ManagedJobClientHelper.getPort(job.getEndpoint());

		Subscribe request = new Subscribe();
		request.setConsumerReference(notificationConsumerEPR);
/* Replacement code taken from GramJob.bind()
		TopicExpressionType topicExpression = 
			new TopicExpressionType(
									WSNConstants.SIMPLE_TOPIC_DIALECT,
									ManagedJobConstants.RP_STATE);
		request.setTopicExpression(topicExpression);	
*/
        TopicExpressionType topicExpression = new TopicExpressionType();
        topicExpression.setDialect(WSNConstants.SIMPLE_TOPIC_DIALECT);
        topicExpression.setValue(
            ManagedJobConstants.STATE_CHANGE_INFORMATION_TOPIC_QNAME);
        MessageElement element =
            (MessageElement) ObjectSerializer.toSOAPElement(
                topicExpression, WSNConstants.TOPIC_EXPRESSION);
        FilterType filter = new FilterType();
        filter.set_any(new MessageElement[] { element });
        request.setFilter(filter);

		GramJobUtils.setDefaultJobAttributes (jobPort, 
											  GSIUtils.getCredential (gahp));
		SubscribeResponse response = jobPort.subscribe (request);
	}


	public void removeJobListener (String jobContact) {
        // TODO Eliminate this function?
        return;
	}

    
    public synchronized void doCleanup() {
        try {
            this.notificationConsumerManager.removeNotificationConsumer(
                                                    notificationConsumerEPR);

            this.notificationConsumerManager.stopListening();
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
        }
    }

    
    public void deliver (List topicPath,
						 EndpointReferenceType producer,
						 Object message)
    {
        String jobContact = null;
		String jobState = null;
        int jobExitCode = -1;
		BaseFaultType[] jobFaults = null;

		try {
            jobContact = ManagedJobClientHelper.getHandle( producer );

            StateChangeNotificationMessageType changeNotification =
                NotificationUtil.getStateChangeNotification(message);
            StateEnumeration state = changeNotification.getState();
            jobState = state.getValue();
			if (state.equals (StateEnumeration.Failed)) {
				jobFaults = FaultUtils.getConcreteFaults(changeNotification.getFault());
			}
            if (state.equals(StateEnumeration.StageOut)
                || state.equals(StateEnumeration.Done)
                || state.equals(StateEnumeration.Failed)) {
                jobExitCode = changeNotification.getExitCode();
            }
        }
        catch (Exception e) {
            System.err.println( "Exception while processing callback" );
            e.printStackTrace(System.err);
            return;
		}

        if ( jobFaults != null ) {
            System.err.println( "Full fault for job " + jobContact + ":" );
            for (int i=0; i<jobFaults.length; i++) {
                System.err.println( FaultUtils.faultToString( jobFaults[i] ) );
            }
        }

		gahp.addResult (
						requestId,
						new String[] {
							jobContact,
							jobState,
							(jobFaults!=null)?FaultUtils.getErrorMessageFromFaults(jobFaults):"NULL",
                            (jobExitCode==-1) ? "NULL" : ""+jobExitCode
                           }
						);

    }

		// This is copied from GramJob.java
    private BaseFaultType getFaultFromRP(FaultResourcePropertyType fault)
    {
        if (fault == null)
        {
            return null;
        }

        if (fault.getFault() != null) {
            return fault.getFault();
        } else if (fault.getCredentialSerializationFault() != null) {
            return fault.getCredentialSerializationFault();
        } else if (fault.getExecutionFailedFault() != null) {
            return fault.getExecutionFailedFault();
        } else if (fault.getFilePermissionsFault() != null) {
            return fault.getFilePermissionsFault();
        } else if (fault.getInsufficientCredentialsFault() != null) {
            return fault.getInsufficientCredentialsFault();
         } else if (fault.getInternalFault() != null) {
            return fault.getInternalFault();
        } else if (fault.getInvalidCredentialsFault() != null) {
            return fault.getInvalidCredentialsFault();
        } else if (fault.getInvalidPathFault() != null) {
            return fault.getInvalidPathFault();
        } else if (fault.getServiceLevelAgreementFault() != null) {
            return fault.getServiceLevelAgreementFault();
        } else if (fault.getStagingFault() != null) {
            return fault.getStagingFault();
        } else if (fault.getUnsupportedFeatureFault() != null) {
            return fault.getUnsupportedFeatureFault();
        } else {
            return null;
        }
    }

    public static void storeCallbackSink (GahpInterface gahp, 
										  String id, 
										  CallbackSink sink) {
        gahp.storeObject (CALLBACK_SINK+id, sink);

			// And add it to the lsit
		List sinkList = getAllCallbackSinks(gahp);
		sinkList.add (sink);
    }
    

    public static CallbackSink getCallbackSink (GahpInterface gahp, 
												String id) {
        return (CallbackSink)gahp.getObject (CALLBACK_SINK+id);
    }


	public static List getAllCallbackSinks (GahpInterface gahp) {
		List list = null;
		synchronized (gahp) {
			list = (List)gahp.getObject (CALLBACK_SINK_LIST);
			if (list == null) {
				list = new Vector();
				gahp.storeObject (CALLBACK_SINK_LIST, list);
			}
		}

		return list;
	}

}
