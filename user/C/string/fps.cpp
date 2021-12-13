using UnityEngine;
using System.Collections;


public class Main : MonoBehaviour
{
	public Texture2D AimPoint;
	// Use this for initialization
	void Start()
	{
		Screen.showCursor = false;//取消鼠标默认图案
	}

	// Update is called once per frame
	void Update()
	{

	}
	void OnGUI()
	{
		//AimPoint.width >> 1和(AimPoint.height >>是右移一位，相当于除以2
		//(x >> 1) 和 (x / 2) 的结果是一样的
		Rect  rect = new Rect(Input.mousePosition.x - (AimPoint.width >> 1),
		                      Screen.height - Input.mousePosition.y - (AimPoint.height >> 1),
		                      AimPoint.width,
		                      AimPoint.height);
		GUI.DrawTexture(rect, AimPoint);
	}
}
