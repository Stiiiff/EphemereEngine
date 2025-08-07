// Copyright Epic Games, Inc. All Rights Reserved.

using AutomationTool;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Xml;
using Tools.DotNETCommon;
using UnrealBuildTool;

namespace BuildGraph.Tasks
{
	/// <summary>
	/// Parameters for a task that notarizes a dmg via the apple notarization process
	/// </summary>
	public class NotarizeTaskParameters
	{
		/// <summary>
		/// Path to the dmg to notarize
		/// </summary>
		[TaskParameter]
		public string DmgPath;

		/// <summary>
		/// primary bundle ID
		/// </summary>
		[TaskParameter]
		public string BundleID;

		/// <summary>
		/// Apple ID Username
		/// </summary>
		[TaskParameter]
		public string UserName;

		/// <summary>
		/// The keychain ID
		/// </summary>
		[TaskParameter]
		public string KeyChainID;

		/// <summary>
		/// When true the notarization ticket will be stapled
		/// </summary>
		[TaskParameter(Optional = true)]
		public bool RequireStapling = false;
	}

	[TaskElement("Notarize", typeof(NotarizeTaskParameters))]
	class NotarizeTask : CustomTask
	{
		/// <summary>
		/// Parameters for the task
		/// </summary>
		NotarizeTaskParameters Parameters;

		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="InParameters">Parameters for the task</param>
		public NotarizeTask(NotarizeTaskParameters InParameters)
		{
			Parameters = InParameters;
		}

		/// <summary>
		/// Execute the task.
		/// </summary>
		/// <param name="Job">Information about the current job</param>
		/// <param name="BuildProducts">Set of build products produced by this node.</param>
		/// <param name="TagNameToFileSet">Mapping from tag names to the set of files they include</param>
		public override void Execute(JobContext Job, HashSet<FileReference> BuildProducts, Dictionary<string, HashSet<FileReference>> TagNameToFileSet)
		{
			// Ensure running on a mac.
			throw new AutomationException("Notarization can only be run on a Mac!");
		}

		private string GetLogFile(string Url)
		{
			HttpWebRequest Request = (HttpWebRequest)WebRequest.Create(Url);
			Request.Method = "GET";
			try
			{
				WebResponse Response = Request.GetResponse();
				string ResponseContent = null;
				using (StreamReader ResponseReader = new System.IO.StreamReader(Response.GetResponseStream(), Encoding.Default))
				{
					ResponseContent = ResponseReader.ReadToEnd();
				}
				return ResponseContent;
			}
			catch (WebException Ex)
			{
				if (Ex.Response != null)
				{
					throw new AutomationException(Ex, string.Format("Request returned status: {0}, message: {1}", ((HttpWebResponse)Ex.Response).StatusCode, Ex.Message));
				}
				else
				{
					throw new AutomationException(Ex, string.Format("Request returned message: {0}", Ex.InnerException.Message));
				}
			}
			catch (Exception Ex)
			{
				throw new AutomationException(Ex, string.Format("Couldn't complete the request, error: {0}", Ex.Message));
			}
		}

		/// <summary>
		/// Output this task out to an XML writer.
		/// </summary>
		public override void Write(XmlWriter Writer)
		{
			Write(Writer, Parameters);
		}

		/// <summary>
		/// Find all the tags which are used as inputs to this task
		/// </summary>
		/// <returns>The tag names which are read by this task</returns>
		public override IEnumerable<string> FindConsumedTagNames()
		{
			yield break;
		}

		/// <summary>
		/// Find all the tags which are modified by this task
		/// </summary>
		/// <returns>The tag names which are modified by this task</returns>
		public override IEnumerable<string> FindProducedTagNames()
		{
			yield break;
		}
	}

}