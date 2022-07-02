import { ElMessage } from "element-plus";

function reportError(error: any):void {
    ElMessage({
        message: error.status + " " + error.statusText + " " + error.message,
        type: "error",
      });
}

export default reportError;