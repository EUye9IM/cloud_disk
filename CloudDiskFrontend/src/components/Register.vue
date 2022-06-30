<template>
  <el-form
    ref="ruleFormRef"
    :model="ruleForm"
    :rules="rules"
    label-width="120px"
    class="demo-ruleForm"
    :size="formSize"
    status-icon
  >
    <el-form-item label="用户名" prop="username">
      <el-input v-model="ruleForm.username" maxlength="16" />
    </el-form-item>
    <el-form-item label="密码" prop="password">
      <el-col :span="11">
        <el-input
          v-model="ruleForm.password"
          type="password"
          maxlength="20"
          autocomplete="off"
        />
      </el-col>
      <el-col :span="4">
        <el-progress
          v-if="passwordStrengthProgressPercentage"
          :percentage="passwordStrengthProgressPercentage"
          :color="passwordStrengthColor"
          :format="passwordStrengthFormat"
        />
      </el-col>
    </el-form-item>
    <el-form-item label="确认密码" prop="confirmPassword">
      <el-input
        v-model="ruleForm.confirmPassword"
        type="password"
        maxlength="20"
        autocomplete="off"
      />
    </el-form-item>
    <el-form-item>
      <el-button type="success" @click="submitForm(ruleFormRef)">注册</el-button>
      <el-button type="primary" @click="$router.push('/login')"
        >我有账号</el-button
      >
    </el-form-item>
  </el-form>
</template>

<script lang="ts" setup>
import { reactive, ref, watch } from "vue";
import axios from "axios";
import { ElMessage, FormInstance, FormRules } from "element-plus";
import { server_url } from "../config";
import router from "../router";

const formSize = ref("default");
const ruleFormRef = ref<FormInstance>();
const ruleForm = reactive({
  username: "",
  password: "",
  confirmPassword: "",
});

const isAlpha = (c: string) => {
  return /^[a-zA-Z]$/.test(c);
};

const validateUsername = (
  rule: any,
  value: string,
  callback: (err?: Error | undefined) => void
) => {
  if (!isAlpha(value.charAt(0)) && value.charAt(0) !== "_") {
    callback(new Error("用户名应以字母或下划线开头"));
  }
  const re = /^[a-zA-Z0-9_]*$/;
  if (!re.test(value)) {
    callback(new Error("用户名只能包含字母、数字与下划线"));
  }
  callback();
};

const isStrongPassword = (password: string) => {
  return /(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[^A-Za-z0-9])(?=.{12,})/.test(
    password
  );
};

const isQualifiedPassword = (password: string) => {
  return (
    password.length >= 12 &&
    (/[a-z]/.test(password) ? 1 : 0) +
      (/[A-Z]/.test(password) ? 1 : 0) +
      (/[0-9]/.test(password) ? 1 : 0) +
      (/[^a-zA-Z0-9]/.test(password) ? 1 : 0) >=
      3
  );
};

const strongPasswordPercentage = 100;
const qualifiedPasswordPercentage = 70;
const weakPasswordpercentage = 40;

const passwordStrengthProgressPercentage = ref(0);

const passwordStrengthFormat = (percentage: number) =>
  percentage === strongPasswordPercentage
    ? "强"
    : percentage === qualifiedPasswordPercentage
    ? "合格"
    : "弱";

const passwordStrengthColor = (percentage: number) =>
  percentage === strongPasswordPercentage
    ? "#67C23A"
    : percentage === qualifiedPasswordPercentage
    ? "#E6A23C"
    : "#F56C6C";

watch(
  () => ruleForm.password,
  (password, _) => {
    if (password.length === 0) {
      passwordStrengthProgressPercentage.value = 0;
    } else if (isStrongPassword(password)) {
      passwordStrengthProgressPercentage.value = strongPasswordPercentage;
    } else if (isQualifiedPassword(password)) {
      passwordStrengthProgressPercentage.value = qualifiedPasswordPercentage;
    } else {
      passwordStrengthProgressPercentage.value = weakPasswordpercentage;
    }
  }
);

const validatePass = (
  rule: any,
  value: string,
  callback: (err?: Error | undefined) => void
) => {
  {
    if (isQualifiedPassword(value)) {
      callback();
    } else {
      callback(
        new Error("密码应至少包含大写字母、小写字母、数字、特殊字符中的三项")
      );
    }
  }
};

const validatePass2 = (
  rule: any,
  value: string,
  callback: (err?: Error | undefined) => void
) => {
  if (value !== ruleForm.password) {
    callback(new Error("两次输入的密码不一致"));
  } else {
    callback();
  }
};

const rules = reactive<FormRules>({
  username: [
    {
      required: true,
      type: "string",
      message: "请输入用户名",
      trigger: "blur",
    },
    { min: 3, max: 16, message: "用户名应当在3-16字符之间", trigger: "blur" },
    { validator: validateUsername, trigger: "blur" },
  ],
  password: [
    { required: true, type: "string", message: "请输入密码", trigger: "blur" },
    { min: 8, max: 20, message: "密码应当在12-20字符之间", trigger: "blur" },
    { validator: validatePass, trigger: "blur" },
  ],
  confirmPassword: [
    {
      required: true,
      type: "string",
      message: "请再次输入密码",
      trigger: "blur",
    },
    { validator: validatePass2, trigger: "blur" },
  ],
});

const submitForm = async (formEl: FormInstance | undefined) => {
  if (!formEl) return;
  await formEl.validate((valid, fields) => {
    if (valid) {
      axios
        .post(
          server_url + "/api/signup",
          JSON.stringify({
            user: ruleForm.username,
            password: ruleForm.password,
            confirmpassword: ruleForm.confirmPassword,
          })
        )
        .then((res) => {
          if (res.data.ret === 0) {
            ElMessage({
              message: "注册成功!",
              type: "success",
            });
            router.push("/home");
          } else {
            ElMessage({
              message: res.data.message,
              type: "error",
            });
          }
        })
        .catch((error) =>
          ElMessage({
            message: "错误：" + error.status + " " + error.message,
            type: "error",
          })
        );
    } else {
      console.log("error submit!", fields);
    }
  });
};
</script>
